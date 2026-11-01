#include "dce.h"
#include <iostream>
#include <set>
#include <cctype>

void eliminateDeadCode(CFG& cfg, TAC& tac) {
    int killed = 0;

    for (auto& blk : cfg.blocks) {
        std::set<std::string> live = blk.OUT;


        for (int i = (int)blk.instrIdx.size() - 1; i >= 0; i--) {
            int idx = blk.instrIdx[i];
            auto& ins = tac[idx];

            bool writesToVar =
                (ins.op == OP_ASSIGN || ins.op == OP_ADD  ||
                 ins.op == OP_SUB    || ins.op == OP_MUL  ||
                 ins.op == OP_DIV    ||
                 ins.op == OP_EQ     || ins.op == OP_NEQ  ||
                 ins.op == OP_LT     || ins.op == OP_GT   ||
                 ins.op == OP_LE     || ins.op == OP_GE);

            if (writesToVar && !ins.result.empty()) {
                if (live.find(ins.result) == live.end()) {
                    ins.dead = true;
                    killed++;
                    continue;
                } else {
                    live.erase(ins.result);
                }
            }

            auto addLive = [&](const std::string& v) {
                if (!v.empty() && !(std::isdigit(v[0]) || v[0] == '-'))
                    live.insert(v);
            };

            switch (ins.op) {
                case OP_ASSIGN:
                    addLive(ins.arg1);
                    break;
                case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV:
                case OP_EQ:  case OP_NEQ: case OP_LT:
                case OP_GT:  case OP_LE:  case OP_GE:
                    addLive(ins.arg1);
                    addLive(ins.arg2);
                    break;
                case OP_JUMPIF: case OP_JUMPIFNOT:
                    addLive(ins.arg1);
                    break;
                case OP_RETURN:
                    addLive(ins.arg1);
                    break;
                default: break;
            }
        }
    }

    std::cout << "\nDCE eliminated " << killed << " instruction(s).\n";
}

void printOptimizedTAC(const TAC& tac) {
    std::cout << "\n=== Optimized TAC (Dead Code Removed) ===\n";
    for (int i = 0; i < (int)tac.size(); i++) {
        const auto& ins = tac[i];
        if (!ins.dead) {
            std::cout << i << ": ";
            switch (ins.op) {
                case OP_ASSIGN:
                    std::cout << ins.result << " = " << ins.arg1; break;
                case OP_ADD:
                    std::cout << ins.result << " = "
                              << ins.arg1 << " + " << ins.arg2; break;
                case OP_SUB:
                    std::cout << ins.result << " = "
                              << ins.arg1 << " - " << ins.arg2; break;
                case OP_MUL:
                    std::cout << ins.result << " = "
                              << ins.arg1 << " * " << ins.arg2; break;
                case OP_DIV:
                    std::cout << ins.result << " = "
                              << ins.arg1 << " / " << ins.arg2; break;
                case OP_EQ:  std::cout << ins.result << " = " << ins.arg1 << " == " << ins.arg2; break;
                case OP_NEQ: std::cout << ins.result << " = " << ins.arg1 << " != " << ins.arg2; break;
                case OP_LT:  std::cout << ins.result << " = " << ins.arg1 << " < "  << ins.arg2; break;
                case OP_GT:  std::cout << ins.result << " = " << ins.arg1 << " > "  << ins.arg2; break;
                case OP_LE:  std::cout << ins.result << " = " << ins.arg1 << " <= " << ins.arg2; break;
                case OP_GE:  std::cout << ins.result << " = " << ins.arg1 << " >= " << ins.arg2; break;
                case OP_RETURN:
                    std::cout << "return " << ins.arg1; break;
                case OP_LABEL:
                    std::cout << ins.result << ":"; break;
                case OP_JUMP:
                    std::cout << "goto " << ins.result; break;
                case OP_JUMPIF:
                    std::cout << "if " << ins.arg1 << " goto " << ins.result; break;
                case OP_JUMPIFNOT:
                    std::cout << "ifFalse " << ins.arg1 << " goto " << ins.result; break;
                default: break;
            }
            std::cout << "\n";
        }
    }
}