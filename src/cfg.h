#pragma once
#include "tac.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <set>

//Basic Block 
struct BasicBlock {
    int              id;         
    std::vector<int> instrIdx;    
    std::vector<int> successors; 
    std::vector<int> predecessors;

    // Liveness sets
    std::set<std::string> USE;   
    std::set<std::string> DEF;  
    std::set<std::string> IN;   
    std::set<std::string> OUT;  
};

//CFG
struct CFG {
    std::vector<BasicBlock>             blocks;
    std::unordered_map<std::string,int> labelToBlock; 

    void build(const TAC& tac);
    void addEdge(int from, int to);
    void print() const;
};