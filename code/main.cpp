/*
* @Author: sun000
* @Date:   2018-05-01 14:29:53
* @Last Modified by:   sun000
* @Last Modified time: 2018-05-01 15:41:06
*/

#include "Parsing.h"

int main(void)  {
     Parse *p = new Parse("InputProgram.txt");
     // p -> justLex();
     p -> parsing();
     // p -> printSyntaxTree(true);
     printf("\n***********************************\n");
     printf("输出语法树在\"SyntaxTree.txt\"文件中!\n");
     printf("***********************************\n\n");
     p -> saveSyntaxTree("SyntaxTree.txt",true);
     return 0;
}