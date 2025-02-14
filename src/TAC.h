#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include "AST.h"

struct TAC {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

TAC(std::string op, std::string arg1, std::string arg2, std::string result)
    : op(op), arg1(arg1), arg2(arg2), result(result) {}

void print() {
    if(arg2.empty()){
        std::cout << result << " = " << op << " " << arg1 << std::endl;
    }else{
        std::cout << result << " = " << arg1 << " " << op << " " << arg2 << std::endl;
    }
}

};