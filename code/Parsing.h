#pragma once

#include <string>
#include <queue>
#include <map>
#include <fstream>
#include <cctype>
#include <set>
#include <vector>
#include <iostream>

struct DFANODE {
    bool canToId = false;
    std::string type;
    DFANODE(bool mcanToId = true, std::string mType = "UNKNOW"): canToId(mcanToId), type(mType) {}
    std::map<char, DFANODE*> edge;

    DFANODE* addEdgeAndGoNext(char c, bool mcanToId);
    DFANODE* getNext(char c, bool needToId);
};

struct TOKEN {
    std::string type;
    std::string* semantic = nullptr;
    TOKEN(std::string mType = "UNKNOW", std::string* mSemantic = nullptr): type(mType), semantic(mSemantic) {}
};

class DFA {
private:
    DFANODE *root = nullptr;
    DFA();
    DFA operator & ();
    DFA(DFA&);

    void add(std::string Vt, bool canToId);
    void DFAInit();
public:
    DFANODE* getRoot() {return root;}
    static DFA* instance;
    void showDFA();
    static DFA* getDFA();
    std::string getTOKEN(std::string & word);
    void checkOneLine(std::string str, std::queue<TOKEN*> &q);
};

class Lexer {
public:
    static int nowLine;
    static std::string nowStr;
    static std::ifstream inputFile;
    static std::queue<TOKEN*> Q;
    static TOKEN* getTOKEN(std::string word);
    static TOKEN* getNextTOKEN(std::string fileName);
    Lexer() {}
    ~Lexer() {
        inputFile.close();
    }
};

struct TreeNode {
    std::string symbol = "";
    int son_Cnt = 0, tmp_Cnt = 0;
    TreeNode* fa  = nullptr;
    std::vector<TreeNode*> son;
    TreeNode(){}
    TreeNode(std::string mSymbol):symbol(mSymbol){}

    void print(int tab, bool flag);
    void save(std::ofstream& fout, int tab, bool flag);
    void deleteAllSon();//递归删除整棵树占用的空间 
};

class SyntaxTree {
private:
    TreeNode *root = nullptr;
public:
    void setRoot(TreeNode* p){ root = p;}
    ~SyntaxTree() {
        if(root) {
            root -> deleteAllSon();
            delete root;
            root = nullptr;
        }
    }
    TreeNode* buildNode(std::string mSymbol);
    void addNode(TreeNode* mFa, TreeNode* mSon); 
    void setNum(TreeNode* p, int num);

    void saveTree(std::string path, bool flag);
    void printTree(bool flag);//参数是是否输出参考线
};

class Parse {
private:    
    SyntaxTree* tree = nullptr;

    std::map<std::string, std::vector<std::set<std::string> > > predictSet;
    std::string fileName;
    std::string token;
    void predictSetInit();
    void error();

    TreeNode* match(std::string par);
    TreeNode* ActParamList();
    TreeNode* ActParamMore();
    TreeNode* AddOp();
    TreeNode* ArrayType();
    TreeNode* AssCall();
    TreeNode* AssignmentRest();
    TreeNode* BaseType();
    TreeNode* CallStmRest();
    TreeNode* CmpOp();
    TreeNode* ConditionalStm();
    TreeNode* DeclarePart();
    TreeNode* Exp();
    TreeNode* Factor();
    TreeNode* FidMore();
    TreeNode* FieldDecList();
    TreeNode* FieldDecMore();
    TreeNode* FieldVar();
    TreeNode* FieldVarMore();
    TreeNode* FormList();
    TreeNode* IdList();
    TreeNode* IdMore();
    TreeNode* InputStm();
    TreeNode* Invar();
    TreeNode* LoopStm();
    TreeNode* Low();
    TreeNode* MultOp();
    TreeNode* OtherFactor();
    TreeNode* OtherRelE();
    TreeNode* OtherTerm();
    TreeNode* OutputStm();
    TreeNode* Param();
    TreeNode* ParamDecList();
    TreeNode* ParamList();
    TreeNode* ParamMore();
    TreeNode* ProcBody();
    TreeNode* ProcDec();
    TreeNode* ProcDecMore();
    TreeNode* ProcDecPart();
    TreeNode* ProcDecpart();
    TreeNode* ProcName();
    TreeNode* Program();
    TreeNode* ProgramBody();
    TreeNode* ProgramHead();
    TreeNode* ProgramName();
    TreeNode* RecType();
    TreeNode* RelExp();
    TreeNode* ReturnStm();
    TreeNode* Stm();
    TreeNode* StmList();
    TreeNode* StmMore();
    TreeNode* StructureType();
    TreeNode* Term();
    TreeNode* Top();
    TreeNode* TypeDec();
    TreeNode* TypeDecList();
    TreeNode* TypeDecMore();
    TreeNode* TypeDecpart();
    TreeNode* TypeDef();
    TreeNode* TypeId();
    TreeNode* VarDec();
    TreeNode* VarDecList();
    TreeNode* VarDecMore();
    TreeNode* VarDecpart();
    TreeNode* VarIdList();
    TreeNode* VarIdMore();
    TreeNode* VariMore();
    TreeNode* Variable();
public:
    Parse(std::string file = ""):fileName(file){ 
        tree = new SyntaxTree();
        predictSetInit(); 
    };
    void parsing();
    void saveSyntaxTree(std::string fileName, bool flag);
    void printSyntaxTree(bool flag);
    void justLex();
    void showPredict() {
        for(auto it = predictSet.begin(); it != predictSet.end(); it++) {
            std::cout << it -> first;
            printf(" %lu\n", (it -> second).size());
        }
    }
};




