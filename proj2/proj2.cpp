#include <stdio.h>
#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#define MAXLINE 1000 /* maximum input line length */


int mgetline(char line[], int max);
int strindex(char source[], char searchfor[]);
char pattern[] = "(";
char newLine = '(';

std::string convertToString(char* a, int size)
{
    std::string s = a;
    return s;
}

std::vector<std::string> split(std::string strToSplit, char delimeter)
{
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter))
    {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}

int main (int argc, char *argv[]) {
    char line[MAXLINE];
    int found = 0;
    std::vector<std::string> myvec;
    
    while (mgetline(line, MAXLINE) > 0)
        if (strindex(line, pattern) >= 0) {
            int size = sizeof(line)/sizeof(char);

            std::string all = convertToString(line,size);
            std::vector<std::string> splitArray = split(all, newLine);
            myvec.push_back(splitArray[0]);
            found++;
        }
    
    std::map<std::string, int> countCall;
    for (auto & elem : myvec){
    	auto result = countCall.insert(std::pair<std::string,int>(elem,1));
    	if(result.second == false)
    		result.first->second++;
    }
    int a = countCall.size();
    printf("AAA: %d invoked system call instances from %d unique system calls.\n", found, a);

    
    
    for (auto & elem : countCall){
   	 if (argc > 1 && strcmp(argv[1],"seq") == 0){
   	 	if(elem.first == "close"||elem.first == "mmap"||elem.first == "mprotect"||elem.first == "arch_prct"){
   	 	std::cout << "mmap" << ":" << elem.first << "  " << elem.second << std::endl;
   	 	}
   	 	if(elem.first == "fstat"||elem.first == "read"){
   	 	std::cout << "open" << ":" << elem.first << "  " << elem.second << std::endl;
   	 	}
   	 	else std::cout << elem.first << ":" << elem.first << "  " << elem.second << std::endl;
   	 }
   	 
    	else std::cout << elem.first << "  " << elem.second << std::endl;
    }
    
    
        
    return found;
}

int mgetline(char s[], int lim)
{
    int c, i;
    i = 0;
    while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
        s[i++] = c;
    if (c == '\n')
        s[i++] = c;
    s[i] = '\0';
    return i;
}
int strindex(char s[], char t[])
{
    int i, j, k;
    for (i = 0; s[i] != '\0'; i++) {
        for (j=i, k=0; t[k]!='\0' && s[j]==t[k]; j++, k++)
            ;
        if (k > 0 && t[k] == '\0')
            return i;
    }
    return -1;
}

