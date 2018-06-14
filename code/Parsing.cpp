/*
* @Author: sun000
* @Date:   2018-04-28 08:58:44
* @Last Modified by:   sun000
* @Last Modified time: 2018-05-01 15:39:13
*/

#include "Parsing.h"
#include <cstdio>
#include <sstream>

DFANODE *IdAlpha; 
DFANODE *IdNum;

DFANODE* DFANODE::addEdgeAndGoNext(char c, bool mcanToId) {
    DFANODE* tmp = new DFANODE(mcanToId);
    edge.insert(std::make_pair(c, tmp));
    return tmp;
}

DFANODE* DFANODE::getNext(char c, bool needToId) {
    if (edge.find(c) != edge.end())
        return edge[c];
    else if (canToId && needToId) {
        if (isalpha(c)) return IdAlpha;
        else if (isdigit(c)) return IdNum;
    }
    return nullptr;
}


DFA* DFA::instance = nullptr;

DFA::DFA() {
    DFAInit();
}

void DFA::add(std::string Vt, bool canToId) {
    DFANODE* p = root, *tp;
    int i;
    for(i = 0; i < Vt.length(); i++) {
        tp = (p -> getNext(Vt[i], false));
        if(tp != nullptr) p = tp;
        else break;
    }
    for(; i < Vt.length(); i++) {
        p = p -> addEdgeAndGoNext(Vt[i], canToId);
        if(canToId) p -> type = "ID";
    }
    transform(Vt.begin(), Vt.end(), Vt.begin(), ::toupper);
    p -> type = Vt;
}

void DFA::DFAInit() {
    //添加字符节点
    DFANODE* mChar1 = new DFANODE(false, "UNKNOW");
    DFANODE* mChar2 = new DFANODE(false, "CHAR");
    mChar1 -> edge['\''] = mChar2;
    char allChar[16] = {'+', '-', '*', '/', '{', '}', '[', ']', '(', ')', '<', '=', '.', ';', ' ', EOF};
    for(int i = 0; i < 26; i++) {
        mChar1 -> edge['a' + i] = mChar1;
        mChar1 -> edge['A' + i] = mChar1;
    }
    for(int i = 0; i < 10; i++) {
        mChar1 -> edge['0' + i] = mChar1;
    }
    for(int i = 0; i < 16; i++) {
        mChar1 -> edge[allChar[i]] = mChar1;
    }
    //标志符
    IdAlpha = new DFANODE(true, "ID");
    IdNum = new DFANODE(true, "ID");
    //无符号数字
    DFANODE* numNode = new DFANODE(false, "INTC");
    //初始化保留字
    std::ifstream fin("保留字.txt");
    if (!fin.is_open()) printf("打开保留字文件失败\n");
    else {
        root = new DFANODE(false, "");
        root -> edge['\''] = mChar1;
        std::string str;
        while(!fin.eof()) {
            fin >> str;
            add(str, true);
        }
        for(int i = 0; i < 26; i++) {
            if(root -> edge.find(char('a' + i)) == root -> edge.end())
                root -> edge.insert(std::make_pair(char('a' + i), IdAlpha));
            if(root -> edge.find(char('A' + i)) == root -> edge.end())
                root -> edge.insert(std::make_pair(char('A' + i), IdAlpha));
        }
        for(int i = 0; i < 10; i++) {
            root -> edge.insert(std::make_pair(char('0' + i), numNode));
            numNode -> edge.insert(std::make_pair(char('0' + i), numNode));
        }
        fin.close();
    }
    fin.open("特殊符号.txt");
    if (!fin.is_open()) printf("打开特殊符号文件失败\n");
    else {
        std::string str;
        while(!fin.eof()) {
            fin >> str;
            add(str, false);
        }
        fin.close();
    }
}

DFA* DFA::getDFA() {
    if(instance == nullptr)
        instance = new DFA();
    return instance;
}

void DFA::showDFA() {//DEBUG的时候输出DFA观察
    DFANODE* f;
    std::queue<DFANODE*> q;
    q.push(root);
    int s = 1;
    while(!q.empty()) {
        for(int i = 0; i < s; i++) {
            f = q.front(); q.pop();
            for(auto it = f -> edge.begin(); it != f -> edge.end(); it++) {
                printf("%c ", it -> first);
                // std::cout << (it -> second) -> type << " ";
                q.push(it -> second);
                if((it -> second) -> type != "UNKNOW")
                    std::cout << "TYPE|" + (it -> second) -> type << "| ";
                else
                    printf("@ ");
            }
        }
        printf("\n");
        s = q.size();
    }
}

std::string DFA::getTOKEN(std::string &word) {
    DFANODE* p = DFA::instance -> getRoot();
    for(int i = 0; p != nullptr && i < word.length(); p = p -> getNext(word[i], true), i++);
    if(p == nullptr) return "ERROR";
    else return p -> type;
}

void DFA::checkOneLine(std::string str, std::queue<TOKEN*> &Q) {
    DFANODE* p = DFA::instance -> getRoot(), *tp;
    bool flag = false;
    char tmps[1000];
    int cnt = 0;
    for(int i = 0; p != nullptr && i < str.length(); i++) {
        if(str[i] == '\0' || str[i] == ' ' || str[i] == '\t') {
            while(i < str.length() && (str[i] == '\0' || str[i] == ' ' || str[i] == '\t')) {
                printf(" ");
                i++;
            }
            if(i < str.length()) i--;
            flag = true;
            tmps[cnt] = '\0';
        }
        if(!flag) {
            tp = p -> getNext(str[i], true);
            if(tp == nullptr) {
                flag = true;
                i--;
            }
            else {
                p = tp;
                tmps[cnt++] = str[i];
            }
        }
        if(flag) {
            if(p -> type == "UNKNOW") {
                std::cout << "\033[31m" << str + " ERROR!" << std::endl << "\033[0m";
                break;
            }
            else {
                if(cnt != 0) {
                    tmps[cnt] = '\0';
                    std::cout << p -> type + " ";
                    Q.push(new TOKEN(p -> type, new std::string(tmps)));
                }
                cnt = 0;
                flag = false;
                p = DFA::instance -> getRoot();
            }
        }
    }
    printf("\n");
}

int Lexer::nowLine = 0;
std::string Lexer::nowStr = "";
std::ifstream Lexer::inputFile;
std::queue<TOKEN*> Lexer::Q;


TOKEN* Lexer::getTOKEN(std::string word) {
    DFA* dfa = DFA::getDFA();
    std::string type = dfa -> getTOKEN(word);
    if(type == "ERROR" || type == "UNKNOW") {
        std::cout << "\033[31m" << word + " " + type + "!" << std::endl << "\033[0m";
    }
    return new TOKEN(type, &word);
}

TOKEN* Lexer::getNextTOKEN(std::string fileName) {
    while(Lexer::Q.empty()) {
        if(!Lexer::inputFile.is_open()) inputFile.open(fileName);
        if(!Lexer::inputFile.is_open()) {
            printf("文件打开失败!\n");
            return nullptr;
        }
        if(inputFile.eof()) {
            printf("文件已经全部处理完!\n");
            return nullptr;
        }
        std::string str;
        std::getline(inputFile, str);
        str = str + ' ';
        nowLine++;
        nowStr = str;
        DFA::getDFA() -> checkOneLine(str, Lexer::Q);
    }

    TOKEN* ret = Lexer::Q.front();
    Lexer::Q.pop();
    return ret;
}
////////////syatex////////////////////////////////
void TreeNode::deleteAllSon() {
    for(int i = 0; i < son.size(); i++) {
        son[i] -> deleteAllSon();
        delete son[i];
    }
    son.clear();
}

void TreeNode::save(std::ofstream& fout, int tab, bool flag) { 
    if(flag) for(int i = 0; i < tab; i++) {
        if(i & 1) fout << " ";
        else fout << "|"; 
    }
    else for(int i = 0; i < tab; i++) fout << " ";
    fout << symbol << std::endl;
    for(int i = 0; i < son.size(); i++)
        son[i] -> save(fout, tab + 2, flag);
}

void TreeNode::print(int tab, bool flag) { 
    if(flag) for(int i = 0; i < tab; i++) printf("%c", i % 2 ? ' ' : '|');
    else for(int i = 0; i < tab; i++) printf(" ");
    std::cout << symbol << std::endl;
    for(int i = 0; i < son.size(); i++)
        son[i] -> print(tab + 2, flag);
}

TreeNode* SyntaxTree::buildNode(std::string mSymbol) {
    // std::cout << mSymbol << std::endl;
    return new TreeNode(mSymbol);
}

void SyntaxTree::addNode(TreeNode* mFa, TreeNode* mSon) {
    if(mSon == nullptr) return;
    (mFa -> son).push_back(mSon);
    mSon -> fa = mFa;
}

void SyntaxTree::setNum(TreeNode* p, int num) {
    p -> son_Cnt = num;
    // p -> son = std::vector<TreeNode*>;
}

void SyntaxTree::saveTree(std::string path, bool flag) {
    std::ofstream fout(path);
    root -> save(fout, 0, flag);
    fout.close();
}

void SyntaxTree::printTree(bool flag) {
    root -> print(0, flag);
}

//Parseing//////////////////
TreeNode* Parse::match(std::string par) {
    // std::cout << "\033[31m" << "par = " + par << "\033[0m" << std::endl;
    if(par != "Epsilon") {
        if(par == token) {
            TreeNode* p = tree -> buildNode(par);
            TOKEN* ptoken = Lexer::getNextTOKEN(fileName);
            if(ptoken != nullptr) token = ptoken -> type;
            return p;
        }
        else exit(-1);
    }
    return nullptr;
}

void Parse::error() {
    std::cout << "\033[31m" << "Parsing Error!" << std::endl;
    std::cout << "Line " << Lexer::nowLine <<  ": " + Lexer::nowStr  << std::endl;
    std::cout << "Error TOKEN : " << token << std::endl << "\033[0m";
    // Lexer::Q.clear();
    exit(-1);
}

void Parse::predictSetInit() {
    std::ifstream fin("PredictSet.txt");
    std::string str, word, left;
    while(std::getline(fin, str)) {
        std::stringstream sin(str);
        std::set<std::string> tmps;
        sin >> left >> word;
        while(sin >> word) {
            if(word == "|") {
                predictSet[left].push_back(tmps);
                tmps.clear();
            } else {
                tmps.insert(word);
            }
        }
    }
    fin.close();
}


TreeNode* Parse::Program() {
    TreeNode *p = tree -> buildNode("Program");
    if (predictSet["Program"][0].find(token) !=  predictSet["Program"][0].end() ) {
        tree -> setNum(p, predictSet["Program"][0].size());
        tree -> addNode(p, ProgramHead());
        tree -> addNode(p, DeclarePart());
        tree -> addNode(p, ProgramBody());
        tree -> addNode(p, match("."));
    }
    else error();
    return p;
}

TreeNode* Parse::ProgramHead() {
    TreeNode *p = tree -> buildNode("ProgramHead");
    if (predictSet["ProgramHead"][0].find(token) !=  predictSet["ProgramHead"][0].end() ) {
        tree -> setNum(p, predictSet["ProgramHead"][0].size());
        tree -> addNode(p, match("PROGRAM"));
        tree -> addNode(p, ProgramName());
    }
    else error();
    return p;
}

TreeNode* Parse::ProgramName() {
    TreeNode *p = tree -> buildNode("ProgramName");
    if (predictSet["ProgramName"][0].find(token) !=  predictSet["ProgramName"][0].end() ) {
        tree -> setNum(p, predictSet["ProgramName"][0].size());
        tree -> addNode(p, match("ID"));
    }
    else error();
    return p;
}

TreeNode* Parse::DeclarePart() {
    TreeNode *p = tree -> buildNode("DeclarePart");
    if (predictSet["DeclarePart"][0].find(token) !=  predictSet["DeclarePart"][0].end() ) {
        tree -> setNum(p, predictSet["DeclarePart"][0].size());
        tree -> addNode(p, TypeDecpart());
        tree -> addNode(p, VarDecpart());
        tree -> addNode(p, ProcDecpart());
    }
    else error();
    return p;
}

TreeNode* Parse::TypeDecpart() {
    TreeNode *p = tree -> buildNode("TypeDecpart");
    if (predictSet["TypeDecpart"][0].find(token) !=  predictSet["TypeDecpart"][0].end() ) {
        tree -> setNum(p, predictSet["TypeDecpart"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["TypeDecpart"][1].find(token) !=  predictSet["TypeDecpart"][1].end() ) {
        tree -> setNum(p, predictSet["TypeDecpart"][1].size());
        tree -> addNode(p, TypeDec());
    }
    else error();
    return p;
}

TreeNode* Parse::TypeDec() {
    TreeNode *p = tree -> buildNode("TypeDec");
    if (predictSet["TypeDec"][0].find(token) !=  predictSet["TypeDec"][0].end() ) {
        tree -> setNum(p, predictSet["TypeDec"][0].size());
        tree -> addNode(p, match("TYPE"));
        tree -> addNode(p, TypeDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::TypeDecList() {
    TreeNode *p = tree -> buildNode("TypeDecList");
    if (predictSet["TypeDecList"][0].find(token) !=  predictSet["TypeDecList"][0].end() ) {
        tree -> setNum(p, predictSet["TypeDecList"][0].size());
        tree -> addNode(p, TypeId());
        tree -> addNode(p, match("="));
        tree -> addNode(p, TypeDef());
        tree -> addNode(p, match(";"));
        tree -> addNode(p, TypeDecMore());
    }
    else error();
    return p;
}

TreeNode* Parse::TypeDecMore() {
    TreeNode *p = tree -> buildNode("TypeDecMore");
    if (predictSet["TypeDecMore"][0].find(token) !=  predictSet["TypeDecMore"][0].end() ) {
        tree -> setNum(p, predictSet["TypeDecMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["TypeDecMore"][1].find(token) !=  predictSet["TypeDecMore"][1].end() ) {
        tree -> setNum(p, predictSet["TypeDecMore"][1].size());
        tree -> addNode(p, TypeDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::TypeId() {
    TreeNode *p = tree -> buildNode("TypeId");
    if (predictSet["TypeId"][0].find(token) !=  predictSet["TypeId"][0].end() ) {
        tree -> setNum(p, predictSet["TypeId"][0].size());
        tree -> addNode(p, match("ID"));
    }
    else error();
    return p;
}

TreeNode* Parse::TypeDef() {
    TreeNode *p = tree -> buildNode("TypeDef");
    if (predictSet["TypeDef"][0].find(token) !=  predictSet["TypeDef"][0].end() ) {
        tree -> setNum(p, predictSet["TypeDef"][0].size());
        tree -> addNode(p, BaseType());
    }
    else if (predictSet["TypeDef"][1].find(token) !=  predictSet["TypeDef"][1].end() ) {
        tree -> setNum(p, predictSet["TypeDef"][1].size());
        tree -> addNode(p, StructureType());
    }
    else if (predictSet["TypeDef"][2].find(token) !=  predictSet["TypeDef"][2].end() ) {
        tree -> setNum(p, predictSet["TypeDef"][2].size());
        tree -> addNode(p, match("ID"));
    }
    else error();
    return p;
}

TreeNode* Parse::BaseType() {
    TreeNode *p = tree -> buildNode("BaseType");
    if (predictSet["BaseType"][0].find(token) !=  predictSet["BaseType"][0].end() ) {
        tree -> setNum(p, predictSet["BaseType"][0].size());
        tree -> addNode(p, match("INTEGER"));
    }
    else if (predictSet["BaseType"][1].find(token) !=  predictSet["BaseType"][1].end() ) {
        tree -> setNum(p, predictSet["BaseType"][1].size());
        tree -> addNode(p, match("CHAR"));
    }
    else error();
    return p;
}

TreeNode* Parse::StructureType() {
    TreeNode *p = tree -> buildNode("StructureType");
    if (predictSet["StructureType"][0].find(token) !=  predictSet["StructureType"][0].end() ) {
        tree -> setNum(p, predictSet["StructureType"][0].size());
        tree -> addNode(p, ArrayType());
    }
    else if (predictSet["StructureType"][1].find(token) !=  predictSet["StructureType"][1].end() ) {
        tree -> setNum(p, predictSet["StructureType"][1].size());
        tree -> addNode(p, RecType());
    }
    else error();
    return p;
}

TreeNode* Parse::ArrayType() {
    TreeNode *p = tree -> buildNode("ArrayType");
    if (predictSet["ArrayType"][0].find(token) !=  predictSet["ArrayType"][0].end() ) {
        tree -> setNum(p, predictSet["ArrayType"][0].size());
        tree -> addNode(p, match("ARRAY"));
        tree -> addNode(p, match("["));
        tree -> addNode(p, Low());
        tree -> addNode(p, match(".."));
        tree -> addNode(p, Top());
        tree -> addNode(p, match("]"));
        tree -> addNode(p, match("OF"));
        tree -> addNode(p, BaseType());
    }
    else error();
    return p;
}

TreeNode* Parse::Low() {
    TreeNode *p = tree -> buildNode("Low");
    if (predictSet["Low"][0].find(token) !=  predictSet["Low"][0].end() ) {
        tree -> setNum(p, predictSet["Low"][0].size());
        tree -> addNode(p, match("INTC"));
    }
    else error();
    return p;
}

TreeNode* Parse::Top() {
    TreeNode *p = tree -> buildNode("Top");
    if (predictSet["Top"][0].find(token) !=  predictSet["Top"][0].end() ) {
        tree -> setNum(p, predictSet["Top"][0].size());
        tree -> addNode(p, match("INTC"));
    }
    else error();
    return p;
}

TreeNode* Parse::RecType() {
    TreeNode *p = tree -> buildNode("RecType");
    if (predictSet["RecType"][0].find(token) !=  predictSet["RecType"][0].end() ) {
        tree -> setNum(p, predictSet["RecType"][0].size());
        tree -> addNode(p, match("RECORD"));
        tree -> addNode(p, FieldDecList());
        tree -> addNode(p, match("END"));
    }
    else error();
    return p;
}

TreeNode* Parse::FieldDecList() {
    TreeNode *p = tree -> buildNode("FieldDecList");
    if (predictSet["FieldDecList"][0].find(token) !=  predictSet["FieldDecList"][0].end() ) {
        tree -> setNum(p, predictSet["FieldDecList"][0].size());
        tree -> addNode(p, BaseType());
        tree -> addNode(p, IdList());
        tree -> addNode(p, match(";"));
        tree -> addNode(p, FieldDecMore());
    }
    else if (predictSet["FieldDecList"][1].find(token) !=  predictSet["FieldDecList"][1].end() ) {
        tree -> setNum(p, predictSet["FieldDecList"][1].size());
        tree -> addNode(p, ArrayType());
        tree -> addNode(p, IdList());
        tree -> addNode(p, match(";"));
        tree -> addNode(p, FieldDecMore());
    }
    else error();
    return p;
}

TreeNode* Parse::FieldDecMore() {
    TreeNode *p = tree -> buildNode("FieldDecMore");
    if (predictSet["FieldDecMore"][0].find(token) !=  predictSet["FieldDecMore"][0].end() ) {
        tree -> setNum(p, predictSet["FieldDecMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["FieldDecMore"][1].find(token) !=  predictSet["FieldDecMore"][1].end() ) {
        tree -> setNum(p, predictSet["FieldDecMore"][1].size());
        tree -> addNode(p, FieldDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::IdList() {
    TreeNode *p = tree -> buildNode("IdList");
    if (predictSet["IdList"][0].find(token) !=  predictSet["IdList"][0].end() ) {
        tree -> setNum(p, predictSet["IdList"][0].size());
        tree -> addNode(p, match("ID"));
        tree -> addNode(p, IdMore());
    }
    else error();
    return p;
}

TreeNode* Parse::IdMore() {
    TreeNode *p = tree -> buildNode("IdMore");
    if (predictSet["IdMore"][0].find(token) !=  predictSet["IdMore"][0].end() ) {
        tree -> setNum(p, predictSet["IdMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["IdMore"][1].find(token) !=  predictSet["IdMore"][1].end() ) {
        tree -> setNum(p, predictSet["IdMore"][1].size());
        tree -> addNode(p, match(","));
        tree -> addNode(p, IdList());
    }
    else error();
    return p;
}

TreeNode* Parse::VarDecpart() {
    TreeNode *p = tree -> buildNode("VarDecpart");
    if (predictSet["VarDecpart"][0].find(token) !=  predictSet["VarDecpart"][0].end() ) {
        tree -> setNum(p, predictSet["VarDecpart"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["VarDecpart"][1].find(token) !=  predictSet["VarDecpart"][1].end() ) {
        tree -> setNum(p, predictSet["VarDecpart"][1].size());
        tree -> addNode(p, VarDec());
    }
    else error();
    return p;
}

TreeNode* Parse::VarDec() {
    TreeNode *p = tree -> buildNode("VarDec");
    if (predictSet["VarDec"][0].find(token) !=  predictSet["VarDec"][0].end() ) {
        tree -> setNum(p, predictSet["VarDec"][0].size());
        tree -> addNode(p, match("VAR"));
        tree -> addNode(p, VarDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::VarDecList() {
    TreeNode *p = tree -> buildNode("VarDecList");
    if (predictSet["VarDecList"][0].find(token) !=  predictSet["VarDecList"][0].end() ) {
        tree -> setNum(p, predictSet["VarDecList"][0].size());
        tree -> addNode(p, TypeDef());
        tree -> addNode(p, VarIdList());
        tree -> addNode(p, match(";"));
        tree -> addNode(p, VarDecMore());
    }
    else error();
    return p;
}

TreeNode* Parse::VarDecMore() {
    TreeNode *p = tree -> buildNode("VarDecMore");
    if (predictSet["VarDecMore"][0].find(token) !=  predictSet["VarDecMore"][0].end() ) {
        tree -> setNum(p, predictSet["VarDecMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["VarDecMore"][1].find(token) !=  predictSet["VarDecMore"][1].end() ) {
        tree -> setNum(p, predictSet["VarDecMore"][1].size());
        tree -> addNode(p, VarDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::VarIdList() {
    TreeNode *p = tree -> buildNode("VarIdList");
    if (predictSet["VarIdList"][0].find(token) !=  predictSet["VarIdList"][0].end() ) {
        tree -> setNum(p, predictSet["VarIdList"][0].size());
        tree -> addNode(p, match("ID"));
        tree -> addNode(p, VarIdMore());
    }
    else error();
    return p;
}

TreeNode* Parse::VarIdMore() {
    TreeNode *p = tree -> buildNode("VarIdMore");
    if (predictSet["VarIdMore"][0].find(token) !=  predictSet["VarIdMore"][0].end() ) {
        tree -> setNum(p, predictSet["VarIdMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["VarIdMore"][1].find(token) !=  predictSet["VarIdMore"][1].end() ) {
        tree -> setNum(p, predictSet["VarIdMore"][1].size());
        tree -> addNode(p, match(","));
        tree -> addNode(p, VarIdList());
    }
    else error();
    return p;
}

TreeNode* Parse::ProcDecpart() {
    TreeNode *p = tree -> buildNode("ProcDecpart");
    if (predictSet["ProcDecpart"][0].find(token) !=  predictSet["ProcDecpart"][0].end() ) {
        tree -> setNum(p, predictSet["ProcDecpart"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["ProcDecpart"][1].find(token) !=  predictSet["ProcDecpart"][1].end() ) {
        tree -> setNum(p, predictSet["ProcDecpart"][1].size());
        tree -> addNode(p, ProcDec());
    }
    else error();
    return p;
}

TreeNode* Parse::ProcDec() {
    TreeNode *p = tree -> buildNode("ProcDec");
    if (predictSet["ProcDec"][0].find(token) !=  predictSet["ProcDec"][0].end() ) {
        tree -> setNum(p, predictSet["ProcDec"][0].size());
        tree -> addNode(p, match("PROCEDURE"));
        tree -> addNode(p, ProcName());
        tree -> addNode(p, match("("));
        tree -> addNode(p, ParamList());
        tree -> addNode(p, match(")"));
        tree -> addNode(p, match(";"));
        tree -> addNode(p, ProcDecPart());
        tree -> addNode(p, ProcBody());
        tree -> addNode(p, ProcDecMore());
    }
    else error();
    return p;
}

TreeNode* Parse::ProcDecMore() {
    TreeNode *p = tree -> buildNode("ProcDecMore");
    if (predictSet["ProcDecMore"][0].find(token) !=  predictSet["ProcDecMore"][0].end() ) {
        tree -> setNum(p, predictSet["ProcDecMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["ProcDecMore"][1].find(token) !=  predictSet["ProcDecMore"][1].end() ) {
        tree -> setNum(p, predictSet["ProcDecMore"][1].size());
        tree -> addNode(p, ProcDec());
    }
    else error();
    return p;
}

TreeNode* Parse::ProcName() {
    TreeNode *p = tree -> buildNode("ProcName");
    if (predictSet["ProcName"][0].find(token) !=  predictSet["ProcName"][0].end() ) {
        tree -> setNum(p, predictSet["ProcName"][0].size());
        tree -> addNode(p, match("ID"));
    }
    else error();
    return p;
}

TreeNode* Parse::ParamList() {
    TreeNode *p = tree -> buildNode("ParamList");
    if (predictSet["ParamList"][0].find(token) !=  predictSet["ParamList"][0].end() ) {
        tree -> setNum(p, predictSet["ParamList"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["ParamList"][1].find(token) !=  predictSet["ParamList"][1].end() ) {
        tree -> setNum(p, predictSet["ParamList"][1].size());
        tree -> addNode(p, ParamDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::ParamDecList() {
    TreeNode *p = tree -> buildNode("ParamDecList");
    if (predictSet["ParamDecList"][0].find(token) !=  predictSet["ParamDecList"][0].end() ) {
        tree -> setNum(p, predictSet["ParamDecList"][0].size());
        tree -> addNode(p, Param());
        tree -> addNode(p, ParamMore());
    }
    else error();
    return p;
}

TreeNode* Parse::ParamMore() {
    TreeNode *p = tree -> buildNode("ParamMore");
    if (predictSet["ParamMore"][0].find(token) !=  predictSet["ParamMore"][0].end() ) {
        tree -> setNum(p, predictSet["ParamMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["ParamMore"][1].find(token) !=  predictSet["ParamMore"][1].end() ) {
        tree -> setNum(p, predictSet["ParamMore"][1].size());
        tree -> addNode(p, match(";"));
        tree -> addNode(p, ParamDecList());
    }
    else error();
    return p;
}

TreeNode* Parse::Param() {
    TreeNode *p = tree -> buildNode("Param");
    if (predictSet["Param"][0].find(token) !=  predictSet["Param"][0].end() ) {
        tree -> setNum(p, predictSet["Param"][0].size());
        tree -> addNode(p, TypeDef());
        tree -> addNode(p, FormList());
    }
    else if (predictSet["Param"][1].find(token) !=  predictSet["Param"][1].end() ) {
        tree -> setNum(p, predictSet["Param"][1].size());
        tree -> addNode(p, match("VAR"));
        tree -> addNode(p, TypeDef());
        tree -> addNode(p, FormList());
    }
    else error();
    return p;
}

TreeNode* Parse::FormList() {
    TreeNode *p = tree -> buildNode("FormList");
    if (predictSet["FormList"][0].find(token) !=  predictSet["FormList"][0].end() ) {
        tree -> setNum(p, predictSet["FormList"][0].size());
        tree -> addNode(p, match("ID"));
        tree -> addNode(p, FidMore());
    }
    else error();
    return p;
}

TreeNode* Parse::FidMore() {
    TreeNode *p = tree -> buildNode("FidMore");
    if (predictSet["FidMore"][0].find(token) !=  predictSet["FidMore"][0].end() ) {
        tree -> setNum(p, predictSet["FidMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["FidMore"][1].find(token) !=  predictSet["FidMore"][1].end() ) {
        tree -> setNum(p, predictSet["FidMore"][1].size());
        tree -> addNode(p, match(","));
        tree -> addNode(p, FormList());
    }
    else error();
    return p;
}

TreeNode* Parse::ProcDecPart() {
    TreeNode *p = tree -> buildNode("ProcDecPart");
    if (predictSet["ProcDecPart"][0].find(token) !=  predictSet["ProcDecPart"][0].end() ) {
        tree -> setNum(p, predictSet["ProcDecPart"][0].size());
        tree -> addNode(p, DeclarePart());
    }
    else error();
    return p;
}

TreeNode* Parse::ProcBody() {
    TreeNode *p = tree -> buildNode("ProcBody");
    if (predictSet["ProcBody"][0].find(token) !=  predictSet["ProcBody"][0].end() ) {
        tree -> setNum(p, predictSet["ProcBody"][0].size());
        tree -> addNode(p, ProgramBody());
    }
    else error();
    return p;
}

TreeNode* Parse::ProgramBody() {
    TreeNode *p = tree -> buildNode("ProgramBody");
    if (predictSet["ProgramBody"][0].find(token) !=  predictSet["ProgramBody"][0].end() ) {
        tree -> setNum(p, predictSet["ProgramBody"][0].size());
        tree -> addNode(p, match("BEGIN"));
        tree -> addNode(p, StmList());
        tree -> addNode(p, match("END"));
    }
    else error();
    return p;
}

TreeNode* Parse::StmList() {
    TreeNode *p = tree -> buildNode("StmList");
    if (predictSet["StmList"][0].find(token) !=  predictSet["StmList"][0].end() ) {
        tree -> setNum(p, predictSet["StmList"][0].size());
        tree -> addNode(p, Stm());
        tree -> addNode(p, StmMore());
    }
    else error();
    return p;
}

TreeNode* Parse::StmMore() {
    TreeNode *p = tree -> buildNode("StmMore");
    if (predictSet["StmMore"][0].find(token) !=  predictSet["StmMore"][0].end() ) {
        tree -> setNum(p, predictSet["StmMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["StmMore"][1].find(token) !=  predictSet["StmMore"][1].end() ) {
        tree -> setNum(p, predictSet["StmMore"][1].size());
        tree -> addNode(p, match(";"));
        tree -> addNode(p, StmList());
    }
    else error();
    return p;
}

TreeNode* Parse::Stm() {
    TreeNode *p = tree -> buildNode("Stm");
    if (predictSet["Stm"][0].find(token) !=  predictSet["Stm"][0].end() ) {
        tree -> setNum(p, predictSet["Stm"][0].size());
        tree -> addNode(p, ConditionalStm());
    }
    else if (predictSet["Stm"][1].find(token) !=  predictSet["Stm"][1].end() ) {
        tree -> setNum(p, predictSet["Stm"][1].size());
        tree -> addNode(p, LoopStm());
    }
    else if (predictSet["Stm"][2].find(token) !=  predictSet["Stm"][2].end() ) {
        tree -> setNum(p, predictSet["Stm"][2].size());
        tree -> addNode(p, InputStm());
    }
    else if (predictSet["Stm"][3].find(token) !=  predictSet["Stm"][3].end() ) {
        tree -> setNum(p, predictSet["Stm"][3].size());
        tree -> addNode(p, OutputStm());
    }
    else if (predictSet["Stm"][4].find(token) !=  predictSet["Stm"][4].end() ) {
        tree -> setNum(p, predictSet["Stm"][4].size());
        tree -> addNode(p, ReturnStm());
    }
    else if (predictSet["Stm"][5].find(token) !=  predictSet["Stm"][5].end() ) {
        tree -> setNum(p, predictSet["Stm"][5].size());
        tree -> addNode(p, match("ID"));
        tree -> addNode(p, AssCall());
    }
    else error();
    return p;
}

TreeNode* Parse::AssCall() {
    TreeNode *p = tree -> buildNode("AssCall");
    if (predictSet["AssCall"][0].find(token) !=  predictSet["AssCall"][0].end() ) {
        tree -> setNum(p, predictSet["AssCall"][0].size());
        tree -> addNode(p, AssignmentRest());
    }
    else if (predictSet["AssCall"][1].find(token) !=  predictSet["AssCall"][1].end() ) {
        tree -> setNum(p, predictSet["AssCall"][1].size());
        tree -> addNode(p, CallStmRest());
    }
    else error();
    return p;
}

TreeNode* Parse::AssignmentRest() {
    TreeNode *p = tree -> buildNode("AssignmentRest");
    if (predictSet["AssignmentRest"][0].find(token) !=  predictSet["AssignmentRest"][0].end() ) {
        tree -> setNum(p, predictSet["AssignmentRest"][0].size());
        tree -> addNode(p, VariMore());
        tree -> addNode(p, match(":="));
        tree -> addNode(p, Exp());
    }
    else error();
    return p;
}

TreeNode* Parse::ConditionalStm() {
    TreeNode *p = tree -> buildNode("ConditionalStm");
    if (predictSet["ConditionalStm"][0].find(token) !=  predictSet["ConditionalStm"][0].end() ) {
        tree -> setNum(p, predictSet["ConditionalStm"][0].size());
        tree -> addNode(p, match("IF"));
        tree -> addNode(p, RelExp());
        tree -> addNode(p, match("THEN"));
        tree -> addNode(p, StmList());
        tree -> addNode(p, match("ELSE"));
        tree -> addNode(p, StmList());
        tree -> addNode(p, match("FI"));
    }
    else error();
    return p;
}

TreeNode* Parse::LoopStm() {
    TreeNode *p = tree -> buildNode("LoopStm");
    if (predictSet["LoopStm"][0].find(token) !=  predictSet["LoopStm"][0].end() ) {
        tree -> setNum(p, predictSet["LoopStm"][0].size());
        tree -> addNode(p, match("WHILE"));
        tree -> addNode(p, RelExp());
        tree -> addNode(p, match("DO"));
        tree -> addNode(p, StmList());
        tree -> addNode(p, match("ENDWH"));
    }
    else error();
    return p;
}

TreeNode* Parse::InputStm() {
    TreeNode *p = tree -> buildNode("InputStm");
    if (predictSet["InputStm"][0].find(token) !=  predictSet["InputStm"][0].end() ) {
        tree -> setNum(p, predictSet["InputStm"][0].size());
        tree -> addNode(p, match("READ"));
        tree -> addNode(p, match("("));
        tree -> addNode(p, Invar());
        tree -> addNode(p, match(")"));
    }
    else error();
    return p;
}

TreeNode* Parse::Invar() {
    TreeNode *p = tree -> buildNode("Invar");
    if (predictSet["Invar"][0].find(token) !=  predictSet["Invar"][0].end() ) {
        tree -> setNum(p, predictSet["Invar"][0].size());
        tree -> addNode(p, match("ID"));
    }
    else error();
    return p;
}

TreeNode* Parse::OutputStm() {
    TreeNode *p = tree -> buildNode("OutputStm");
    if (predictSet["OutputStm"][0].find(token) !=  predictSet["OutputStm"][0].end() ) {
        tree -> setNum(p, predictSet["OutputStm"][0].size());
        tree -> addNode(p, match("WRITE"));
        tree -> addNode(p, match("("));
        tree -> addNode(p, Exp());
        tree -> addNode(p, match(")"));
    }
    else error();
    return p;
}

TreeNode* Parse::ReturnStm() {
    TreeNode *p = tree -> buildNode("ReturnStm");
    if (predictSet["ReturnStm"][0].find(token) !=  predictSet["ReturnStm"][0].end() ) {
        tree -> setNum(p, predictSet["ReturnStm"][0].size());
        tree -> addNode(p, match("RETURN"));
    }
    else error();
    return p;
}

TreeNode* Parse::CallStmRest() {
    TreeNode *p = tree -> buildNode("CallStmRest");
    if (predictSet["CallStmRest"][0].find(token) !=  predictSet["CallStmRest"][0].end() ) {
        tree -> setNum(p, predictSet["CallStmRest"][0].size());
        tree -> addNode(p, match("("));
        tree -> addNode(p, ActParamList());
        tree -> addNode(p, match(")"));
    }
    else error();
    return p;
}

TreeNode* Parse::ActParamList() {
    TreeNode *p = tree -> buildNode("ActParamList");
    if (predictSet["ActParamList"][0].find(token) !=  predictSet["ActParamList"][0].end() ) {
        tree -> setNum(p, predictSet["ActParamList"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["ActParamList"][1].find(token) !=  predictSet["ActParamList"][1].end() ) {
        tree -> setNum(p, predictSet["ActParamList"][1].size());
        tree -> addNode(p, Exp());
        tree -> addNode(p, ActParamMore());
    }
    else error();
    return p;
}

TreeNode* Parse::ActParamMore() {
    TreeNode *p = tree -> buildNode("ActParamMore");
    if (predictSet["ActParamMore"][0].find(token) !=  predictSet["ActParamMore"][0].end() ) {
        tree -> setNum(p, predictSet["ActParamMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["ActParamMore"][1].find(token) !=  predictSet["ActParamMore"][1].end() ) {
        tree -> setNum(p, predictSet["ActParamMore"][1].size());
        tree -> addNode(p, match(","));
        tree -> addNode(p, ActParamList());
    }
    else error();
    return p;
}

TreeNode* Parse::RelExp() {
    TreeNode *p = tree -> buildNode("RelExp");
    if (predictSet["RelExp"][0].find(token) !=  predictSet["RelExp"][0].end() ) {
        tree -> setNum(p, predictSet["RelExp"][0].size());
        tree -> addNode(p, Exp());
        tree -> addNode(p, OtherRelE());
    }
    else error();
    return p;
}

TreeNode* Parse::OtherRelE() {
    TreeNode *p = tree -> buildNode("OtherRelE");
    if (predictSet["OtherRelE"][0].find(token) !=  predictSet["OtherRelE"][0].end() ) {
        tree -> setNum(p, predictSet["OtherRelE"][0].size());
        tree -> addNode(p, CmpOp());
        tree -> addNode(p, Exp());
    }
    else error();
    return p;
}

TreeNode* Parse::Exp() {
    TreeNode *p = tree -> buildNode("Exp");
    if (predictSet["Exp"][0].find(token) !=  predictSet["Exp"][0].end() ) {
        tree -> setNum(p, predictSet["Exp"][0].size());
        tree -> addNode(p, Term());
        tree -> addNode(p, OtherTerm());
    }
    else error();
    return p;
}

TreeNode* Parse::OtherTerm() {
    TreeNode *p = tree -> buildNode("OtherTerm");
    if (predictSet["OtherTerm"][0].find(token) !=  predictSet["OtherTerm"][0].end() ) {
        tree -> setNum(p, predictSet["OtherTerm"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["OtherTerm"][1].find(token) !=  predictSet["OtherTerm"][1].end() ) {
        tree -> setNum(p, predictSet["OtherTerm"][1].size());
        tree -> addNode(p, AddOp());
        tree -> addNode(p, Exp());
    }
    else error();
    return p;
}

TreeNode* Parse::Term() {
    TreeNode *p = tree -> buildNode("Term");
    if (predictSet["Term"][0].find(token) !=  predictSet["Term"][0].end() ) {
        tree -> setNum(p, predictSet["Term"][0].size());
        tree -> addNode(p, Factor());
        tree -> addNode(p, OtherFactor());
    }
    else error();
    return p;
}

TreeNode* Parse::OtherFactor() {
    TreeNode *p = tree -> buildNode("OtherFactor");
    if (predictSet["OtherFactor"][0].find(token) !=  predictSet["OtherFactor"][0].end() ) {
        tree -> setNum(p, predictSet["OtherFactor"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["OtherFactor"][1].find(token) !=  predictSet["OtherFactor"][1].end() ) {
        tree -> setNum(p, predictSet["OtherFactor"][1].size());
        tree -> addNode(p, MultOp());
        tree -> addNode(p, Term());
    }
    else error();
    return p;
}

TreeNode* Parse::Factor() {
    TreeNode *p = tree -> buildNode("Factor");
    if (predictSet["Factor"][0].find(token) !=  predictSet["Factor"][0].end() ) {
        tree -> setNum(p, predictSet["Factor"][0].size());
        tree -> addNode(p, match("("));
        tree -> addNode(p, Exp());
        tree -> addNode(p, match(")"));
    }
    else if (predictSet["Factor"][1].find(token) !=  predictSet["Factor"][1].end() ) {
        tree -> setNum(p, predictSet["Factor"][1].size());
        tree -> addNode(p, match("INTC"));
    }
    else if (predictSet["Factor"][2].find(token) !=  predictSet["Factor"][2].end() ) {
        tree -> setNum(p, predictSet["Factor"][2].size());
        tree -> addNode(p, Variable());
    }
    else error();
    return p;
}

TreeNode* Parse::Variable() {
    TreeNode *p = tree -> buildNode("Variable");
    if (predictSet["Variable"][0].find(token) !=  predictSet["Variable"][0].end() ) {
        tree -> setNum(p, predictSet["Variable"][0].size());
        tree -> addNode(p, match("ID"));
        tree -> addNode(p, VariMore());
    }
    else error();
    return p;
}

TreeNode* Parse::VariMore() {
    TreeNode *p = tree -> buildNode("VariMore");
    if (predictSet["VariMore"][0].find(token) !=  predictSet["VariMore"][0].end() ) {
        tree -> setNum(p, predictSet["VariMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["VariMore"][1].find(token) !=  predictSet["VariMore"][1].end() ) {
        tree -> setNum(p, predictSet["VariMore"][1].size());
        tree -> addNode(p, match("["));
        tree -> addNode(p, Exp());
        tree -> addNode(p, match("]"));
    }
    else if (predictSet["VariMore"][2].find(token) !=  predictSet["VariMore"][2].end() ) {
        tree -> setNum(p, predictSet["VariMore"][2].size());
        tree -> addNode(p, match("."));
        tree -> addNode(p, FieldVar());
    }
    else error();
    return p;
}

TreeNode* Parse::FieldVar() {
    TreeNode *p = tree -> buildNode("FieldVar");
    if (predictSet["FieldVar"][0].find(token) !=  predictSet["FieldVar"][0].end() ) {
        tree -> setNum(p, predictSet["FieldVar"][0].size());
        tree -> addNode(p, match("ID"));
        tree -> addNode(p, FieldVarMore());
    }
    else error();
    return p;
}

TreeNode* Parse::FieldVarMore() {
    TreeNode *p = tree -> buildNode("FieldVarMore");
    if (predictSet["FieldVarMore"][0].find(token) !=  predictSet["FieldVarMore"][0].end() ) {
        tree -> setNum(p, predictSet["FieldVarMore"][0].size());
        tree -> addNode(p, match("Epsilon"));
    }
    else if (predictSet["FieldVarMore"][1].find(token) !=  predictSet["FieldVarMore"][1].end() ) {
        tree -> setNum(p, predictSet["FieldVarMore"][1].size());
        tree -> addNode(p, match("["));
        tree -> addNode(p, Exp());
        tree -> addNode(p, match("]"));
    }
    else error();
    return p;
}

TreeNode* Parse::CmpOp() {
    TreeNode *p = tree -> buildNode("CmpOp");
    if (predictSet["CmpOp"][0].find(token) !=  predictSet["CmpOp"][0].end() ) {
        tree -> setNum(p, predictSet["CmpOp"][0].size());
        tree -> addNode(p, match("<"));
    }
    else if (predictSet["CmpOp"][1].find(token) !=  predictSet["CmpOp"][1].end() ) {
        tree -> setNum(p, predictSet["CmpOp"][1].size());
        tree -> addNode(p, match("="));
    }
    else error();
    return p;
}

TreeNode* Parse::AddOp() {
    TreeNode *p = tree -> buildNode("AddOp");
    if (predictSet["AddOp"][0].find(token) !=  predictSet["AddOp"][0].end() ) {
        tree -> setNum(p, predictSet["AddOp"][0].size());
        tree -> addNode(p, match("+"));
    }
    else if (predictSet["AddOp"][1].find(token) !=  predictSet["AddOp"][1].end() ) {
        tree -> setNum(p, predictSet["AddOp"][1].size());
        tree -> addNode(p, match("-"));
    }
    else error();
    return p;
}

TreeNode* Parse::MultOp() {
    TreeNode *p = tree -> buildNode("MultOp");
    if (predictSet["MultOp"][0].find(token) !=  predictSet["MultOp"][0].end() ) {
        tree -> setNum(p, predictSet["MultOp"][0].size());
        tree -> addNode(p, match("*"));
    }
    else if (predictSet["MultOp"][1].find(token) !=  predictSet["MultOp"][1].end() ) {
        tree -> setNum(p, predictSet["MultOp"][1].size());
        tree -> addNode(p, match("/"));
    }
    else error();
    return p;
}

void Parse::saveSyntaxTree(std::string fileName, bool flag) {
    tree -> saveTree(fileName, flag);//是否输出参考线
}

void Parse::printSyntaxTree(bool flag) {
    tree -> printTree(flag);
}

void Parse::parsing() {
    token = Lexer::getNextTOKEN(fileName) -> type;
    tree -> setRoot(Program());
    printf("\n***********\n");
    printf("语法检查成功!\n");
    printf("没有语法错误!\n");
    printf("***********\n");
}

void Parse::justLex() {
    while(Lexer::getNextTOKEN("InputProgram.txt"));
}

