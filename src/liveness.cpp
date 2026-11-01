#include "liveness.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <set>

// Helper
static bool isVariable(const std::string& s) {
    if (s.empty()) return false;
    // Constants are pure digits (possibly with a dot)
    return !(std::isdigit(s[0]) || s[0] == '-');
}


static bool isTempName(const std::string& s) {
    if (s.size() < 2 || s[0] != 't') return false;
    return std::all_of(s.begin() + 1, s.end(),
                       [](unsigned char c) { return std::isdigit(c); });
}


void computeUseDef(CFG& cfg, const TAC& tac) {
    for (auto& blk : cfg.blocks) {
        blk.USE.clear();
        blk.DEF.clear();


        for (int idx : blk.instrIdx) {
            const auto& ins = tac[idx];


            auto addUse = [&](const std::string& var) {
                if (isVariable(var) && blk.DEF.find(var) == blk.DEF.end())
                    blk.USE.insert(var);
            };

            switch (ins.op) {
                case OP_ASSIGN:
                    addUse(ins.arg1);   // rhs
                    break;
                case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV:
                case OP_EQ:  case OP_NEQ: case OP_LT:
                case OP_GT:  case OP_LE:  case OP_GE:
                    addUse(ins.arg1);
                    addUse(ins.arg2);
                    break;
                case OP_JUMPIF: case OP_JUMPIFNOT:
                    addUse(ins.arg1);   // condition variable
                    break;
                case OP_RETURN:
                    addUse(ins.arg1);   // returned value
                    break;
                default: break;
            }


            bool writesToResult =
                (ins.op == OP_ASSIGN || ins.op == OP_ADD  ||
                 ins.op == OP_SUB    || ins.op == OP_MUL  ||
                 ins.op == OP_DIV    || ins.op == OP_CALL ||
                 ins.op == OP_EQ     || ins.op == OP_NEQ  ||
                 ins.op == OP_LT     || ins.op == OP_GT   ||
                 ins.op == OP_LE     || ins.op == OP_GE);

            if (writesToResult && isVariable(ins.result)) {
                blk.DEF.insert(ins.result);
            }
        }
    }
}


void computeLiveness(CFG& cfg) {
    int n = (int)cfg.blocks.size();


    std::set<std::string> boundaryLive;
    for (const auto& blk : cfg.blocks) {
        for (const auto& v : blk.DEF) {
            if (isVariable(v) && !isTempName(v))
                boundaryLive.insert(v);
        }
    }

    // Initialize IN and OUT to empty
    for (auto& blk : cfg.blocks) {
        blk.IN.clear();
        blk.OUT.clear();
    }

    bool changed = true;
    int  iterations = 0;

    while (changed) {
        changed = false;
        iterations++;


        for (int b = n - 1; b >= 0; b--) {
            auto& blk = cfg.blocks[b];

            //OUT[B] = ⋃ IN[S] for all successors S
            std::set<std::string> newOut;
            for (int succ : blk.successors) {
                const auto& succIn = cfg.blocks[succ].IN;
                newOut.insert(succIn.begin(), succIn.end());
            }

  
            if (blk.successors.empty()) {
                newOut.insert(boundaryLive.begin(), boundaryLive.end());
            }

            //IN[B] = USE[B] ∪ (OUT[B] − DEF[B])
            std::set<std::string> newIn = blk.USE;
            for (const auto& var : newOut) {
                if (blk.DEF.find(var) == blk.DEF.end())
                    newIn.insert(var);   // OUT - DEF
            }

            // Check for change
            if (newIn != blk.IN || newOut != blk.OUT) {
                blk.IN  = std::move(newIn);
                blk.OUT = std::move(newOut);
                changed = true;
            }
        }
    }
    std::cout << "Liveness converged in " << iterations << " iteration(s).\n";
}

void printLiveness(const CFG& cfg) {
    std::cout << "\n=== Liveness Analysis ===\n";
    for (const auto& blk : cfg.blocks) {
        std::cout << "Block " << blk.id << ":\n";

        auto printSet = [](const std::string& name,
                           const std::set<std::string>& s) {
            std::cout << "  " << name << " = { ";
            for (const auto& v : s) std::cout << v << " ";
            std::cout << "}\n";
        };

        printSet("USE ", blk.USE);
        printSet("DEF ", blk.DEF);
        printSet("IN  ", blk.IN);
        printSet("OUT ", blk.OUT);
    }
}