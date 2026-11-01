#include "tac.h"
#include <iostream>
#include <iomanip>

static const char* opNames[] = {
    "ASSIGN","ADD","SUB","MUL","DIV",
    "EQ","NEQ","LT","GT","LE","GE",
    "LABEL","JUMP","JUMPIF","JUMPIFNOT",
    "RETURN","PARAM","CALL","NOP"
};

void TACBuilder::print() const {
    std::cout << "\n=== Three-Address Code ===\n";
    for (int i = 0; i < (int)instructions.size(); i++) {
        const auto& ins = instructions[i];
        std::cout << std::setw(3) << i << ": ";
        if (ins.dead) std::cout << "[DEAD] ";

        switch (ins.op) {
            case OP_LABEL:
                std::cout << ins.result << ":";
                break;
            case OP_ASSIGN:
                std::cout << ins.result << " = " << ins.arg1;
                break;
            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: {
                char sym = (ins.op==OP_ADD)?'+':
                           (ins.op==OP_SUB)?'-':
                           (ins.op==OP_MUL)?'*':'/';
                std::cout << ins.result << " = " << ins.arg1
                          << " " << sym << " " << ins.arg2;
                break;
            }
            case OP_EQ: case OP_NEQ: case OP_LT:
            case OP_GT: case OP_LE:  case OP_GE: {
                const char* sym = (ins.op==OP_EQ)?"==":
                                  (ins.op==OP_NEQ)?"!=":
                                  (ins.op==OP_LT)?"<":
                                  (ins.op==OP_GT)?">":
                                  (ins.op==OP_LE)?"<=":">=";
                std::cout << ins.result << " = " << ins.arg1
                          << " " << sym << " " << ins.arg2;
                break;
            }
            case OP_JUMP:
                std::cout << "goto " << ins.result;
                break;
            case OP_JUMPIF:
                std::cout << "if " << ins.arg1 << " goto " << ins.result;
                break;
            case OP_JUMPIFNOT:
                std::cout << "ifFalse " << ins.arg1 << " goto " << ins.result;
                break;
            case OP_RETURN:
                std::cout << "return " << ins.arg1;
                break;
            case OP_NOP:
                std::cout << "nop";
                break;
            default:
                std::cout << opNames[ins.op] << " "
                          << ins.result << " " << ins.arg1 << " " << ins.arg2;
        }
        std::cout << "\n";
    }
}