/*
* @Author: sun000
* @Date:   2018-04-29 10:07:03
* @Last Modified by:   sun000
* @Last Modified time: 2018-04-29 13:53:35
*/

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>

using namespace std;

void tidy() {
    ifstream fin("原文法表达式.txt");
    ofstream fout("文法表达式.txt");
    string str, word;
    string pre = "";
    while(!fin.eof()) {
        getline(fin, str);
        stringstream strin(str);
        strin >> word;
        if(word != pre) {
            pre = word;
            fout << "|" << endl << word << " ::= ";
        }
        else fout << "| ";
        while(strin >> word) {
            if(word == "::=") continue;
            fout << word  + " ";
        }
    }
    fin.close();
    fout.close();
}

void classify() {
    ifstream fin("文法表达式.txt");
    set<string> Vn, Vt_Vn;
    string str;
    while(!fin.eof()) {
        getline(fin, str);
        stringstream sin(str);
        sin >> str;
        Vn.insert(str);
        while(sin >> str) {
            if(str != "|" && str != "::=")
                Vt_Vn.insert(str);
        }
    }
    fin.close();

    ofstream VnOut("Vn.txt");
    for(auto it = Vn.begin(); it != Vn.end(); it++) 
        VnOut << *it << endl;
    VnOut.close();

    ofstream VtOut("Vt.txt");
    for(auto it = Vt_Vn.begin(); it != Vt_Vn.end(); it++) {
        if(Vn.find(*it) == Vn.end())
            VtOut << *it << endl;
    }
    VtOut.close();
}

set<string> Vn, Vt;
void setInit() {
    ifstream fin("Vn.txt");
    string str;
    while(fin >> str) {
        Vn.insert(str);
        cout << "void " + str + "();" << endl;
    }
    fin.close();
    fin.open("Vt.txt");
    while(fin >> str) Vt.insert(str);
    fin.close();
}

void oneExpMaker(string left, vector<string> v, ofstream &fout, int id) {
    // transform(left.begin(), left.end(), left.begin(), ::toupper);
    if(id != 0) fout << "else ";
    fout << "if(predictSet[\""+ left + "\"][" <<  id << "].find(token) !=  predictSet[\""+ left +"\"][" << id <<  "].end() ) {" << endl;
    fout << "tree -> setNum(p, predictSet[\""+ left + "\"][" <<  id << "].size());" << endl;
    for(int i = 0; i < v.size(); i++) {
        if(Vn.find(v[i]) != Vn.end()) 
            fout << "tree -> addNode(p, " << v[i] + "());" << endl;
        else 
            fout << "tree -> addNode(p, match(\"" + v[i] + "\"));" << endl;
    }
    fout << "}" << endl;
}

void oneGroupExpMaker(string str, ofstream &fout) {
    stringstream sin(str);
    string word;
    sin >> word;
    string left = word;
    fout << "TreeNode* Parse::" + word + "() { " << endl;
    fout << "TreeNode *p = tree -> buildNode(\"" + word + "\"); " << endl;
    // fout << "std::cout << \"" + word + "\""  + " << std::endl;" << endl;
    sin >> word;
    vector<string> v;
    int id = 0;
    while(sin >> word) {
        if(word == "|") {
            oneExpMaker(left, v, fout, id);   
            id++;
            v.clear();
        }
        else v.push_back(word);
    }
    fout << "else error();" << endl;
    fout << "return p;" << endl;
    fout << "}" << endl << endl;
}


map<string, set<string> > firstSet, followSet;

void oneExpFirst(string str) {
    stringstream sin(str);
    string word, left;
    sin >> left >> word >> word;
    firstSet[left].insert(word);
    bool flag = false;
    while(sin >> word) {
        if(flag && word != "|") firstSet[left].insert(word);
        if(word == "|") flag = true;
    }
}

void getFirst() {//需要手动修改
    setInit();
    ifstream fin("文法表达式.txt");
    ofstream fout("FirstSet.txt", ios::app);
    string str;
    while(!fin.eof()) {
        getline(fin, str);
        oneExpFirst(str);
    }
    // printf("END!\n");
    bool flag = true;
    while(flag) {
        flag = false;
        for(auto l = firstSet.begin(); l != firstSet.end(); l++) {
            string left = l -> first;
            set<string> tmp;
            for(auto it = (l -> second).begin(); it != (l -> second).end(); it++) {
                if(Vt.find(*it) == Vt.end()) {
                    tmp.insert(*it);
                    for(auto it2 = firstSet[*it].begin(); it2 != firstSet[*it].end(); it2++)
                         firstSet[left].insert(*it2);
                }
                else 
                    firstSet[left].insert(*it);
            }
            for(auto it = tmp.begin(); it != tmp.end(); it++) {
                firstSet[left].erase(*it);
                flag = true;
            }
        }
    }
    for(auto it = firstSet.begin(); it != firstSet.end(); it++) {
        string left = it -> first;
        fout << left << "\t::= ";
        for(auto it2 = firstSet[left].begin(); it2 != firstSet[left].end(); it2++) 
            fout << *it2 + " ";
        fout << endl;
    }
    fin.close();
    fout.close(); 
}

map<string, bool> haveE;

void readFirst() {
    haveE.clear();
    ifstream fin("FirstSet.txt");
    string str, word, left;
    while(getline(fin, str)) {
        stringstream sin(str);
        sin >> left >> word;
        haveE[left] = false;
        while(sin >> word) {
            firstSet[left].insert(word);
            if(word == "Epsilon") haveE[left] = true;
        }
    }
    fin.close();
}

bool oneGroupFollow(string str) {
    bool flag = false;
    stringstream sin(str);
    string left, word, pre;
    sin >> left >> word;
    sin >> pre;
    while(sin >> word) {
        if(Vn.find(pre) != Vn.end()) {
            long long int psize =  followSet[pre].size();
            if(word == "|") {
                for(auto it = followSet[left].begin(); it != followSet[left].end(); it++)
                    followSet[pre].insert(*it);
            } else if(Vt.find(word) != Vt.end()) {
                followSet[pre].insert(word);
            } else {
                for(auto it = firstSet[word].begin(); it != firstSet[word].end(); it++)
                    followSet[pre].insert(*it);
                if(haveE[word]) {
                    for(auto it = followSet[left].begin(); it != followSet[left].end(); it++)
                        followSet[pre].insert(*it);
                }
            }
            if(psize != followSet[pre].size()) {
                flag = true;
                // printf("%lld %lu\n", psize, followSet[pre].size());
            }
        }
        if(word == "|") {
            sin >> pre;
        } else {
            pre = word;
        }
    }
    return flag;
}

void getFollow() {
    setInit();
    readFirst();
    followSet.clear();
    followSet["Program"].insert("#");
    bool flag = true;
    // int c = 0;
    while(flag) {
        flag = false;
        // printf("%d ", c); c++;
        ifstream fin("文法表达式.txt");
        string str;
        while(!fin.eof()) {
            getline(fin, str);
            flag |= oneGroupFollow(str);
        }
        // printf("\n");
        fin.close();
    }

    ofstream fout("FollowSet.txt");
    for(auto it = followSet.begin(); it != followSet.end(); it++) {
        string left = it -> first;
        fout << left + " ::= ";
        for(auto it2 = (it -> second).begin(); it2 != (it -> second).end(); it2++) {
            if(*it2 != "Epsilon") 
                fout << *it2 + " ";
        }
        fout << endl;
    }
    fout.close();
}

void readFollow() {
    followSet.clear();
    ifstream fin("FollowSet.txt");
    string str, word, left;
    while(getline(fin, str)) {
        stringstream sin(str);
        sin >> left >> word;
        while(sin >> word) {
            followSet[left].insert(word);
        }
    }
    fin.close();
}

void predictOneGroup(string str, ofstream & fout) {
    stringstream sin(str);
    string left, word;
    sin >> left >> word;
    fout << left << " ::= ";
    set<string> predictSet;
    while(sin >> word) {
        predictSet.clear();
        if(Vt.find(word) != Vt.end()) {
            if(word == "Epsilon") {
                for(auto it = followSet[left].begin(); it != followSet[left].end(); it++)
                    predictSet.insert(*it);
            }
            else 
                predictSet.insert(word);
        }
        else {
            if(haveE[word]) {
                for(auto it = firstSet[word].begin(); it != firstSet[word].end(); it++) {
                    if(*it != "Epsilon")
                        predictSet.insert(*it);
                }
                for(auto it = followSet[left].begin(); it != followSet[left].end(); it++)
                    predictSet.insert(*it);
            } else {
                predictSet = firstSet[word];
            }
        }

        for(auto it = predictSet.begin(); it != predictSet.end(); it++)
            fout << *it + " ";
        fout << " | ";
        while(word != "|") sin >> word;
    }
    fout << endl;
}

void getPredict() {
    setInit();
    readFirst();
    readFollow();
    ifstream fin("文法表达式.txt");
    ofstream fout("PredictSet.txt", ios::app);
    string str;
    while(getline(fin, str)) {
        predictOneGroup(str, fout);
    }
    fin.close();
    fout.close();
}

void programMaker() {
    setInit();
    ifstream fin("文法表达式.txt");
    ofstream fout("自顶向下代码.cpp", ios::app);
    string str;
    while(!fin.eof()) {
        getline(fin, str);
        oneGroupExpMaker(str, fout);
    }
    fin.close();
    fout.close(); 
}

int main(void) {
    programMaker();
    return 0;
}






