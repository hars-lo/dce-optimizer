#include "cfg.h"
#include <iostream>
#include <algorithm>

//Step 1: Identify leaders (first instruction of each basic block) 
//   A leader is:
//     (a) the first instruction
//     (b) any instruction that is the target of a jump
//     (c) any instruction immediately following a jump/return
static std::vector<bool> findLeaders(const TAC& tac) {
    std::vector<bool> isLeader(tac.size(), false);
    if (tac.empty()) return isLeader;

    isLeader[0] = true;

    for (int i = 0; i < (int)tac.size(); i++) {
        const auto& ins = tac[i];

        // Labels are leaders
        if (ins.op == OP_LABEL) {
            isLeader[i] = true;
        }

        // Instruction after a jump or return is a leader
        if (ins.op == OP_JUMP || ins.op == OP_JUMPIF  ||
            ins.op == OP_JUMPIFNOT || ins.op == OP_RETURN) {
            if (i + 1 < (int)tac.size())
                isLeader[i + 1] = true;
        }
    }
    return isLeader;
}

void CFG::build(const TAC& tac) {
    if (tac.empty()) return;

    //Pass 1: partition instructions into basic blocks
    auto isLeader = findLeaders(tac);
    int currentBlock = -1;

    for (int i = 0; i < (int)tac.size(); i++) {
        if (isLeader[i]) {
            blocks.push_back(BasicBlock{});
            currentBlock = (int)blocks.size() - 1;
            blocks[currentBlock].id = currentBlock;

            // If this instruction is a LABEL, record the mapping
            if (tac[i].op == OP_LABEL) {
                labelToBlock[tac[i].result] = currentBlock;
            }
        }
        blocks[currentBlock].instrIdx.push_back(i);
    }

    //Pass 2: add CFG edges
    for (int b = 0; b < (int)blocks.size(); b++) {
        const auto& blk = blocks[b];
        if (blk.instrIdx.empty()) continue;

        int lastIdx  = blk.instrIdx.back();
        const auto& last = tac[lastIdx];

        if (last.op == OP_JUMP) {
            // Unconditional: edge to jump target only
            auto it = labelToBlock.find(last.result);
            if (it != labelToBlock.end())
                addEdge(b, it->second);

        } else if (last.op == OP_JUMPIF || last.op == OP_JUMPIFNOT) {
            // Conditional: edge to target AND fall-through
            auto it = labelToBlock.find(last.result);
            if (it != labelToBlock.end())
                addEdge(b, it->second);           // taken
            if (b + 1 < (int)blocks.size())
                addEdge(b, b + 1);                // fall-through

        } else if (last.op != OP_RETURN) {
            // Sequential: fall through to next block
            if (b + 1 < (int)blocks.size())
                addEdge(b, b + 1);
        }
        // OP_RETURN → no outgoing edges
    }
}

void CFG::addEdge(int from, int to) {
    blocks[from].successors.push_back(to);
    blocks[to].predecessors.push_back(from);
}

void CFG::print() const {
    std::cout << "\n=== Control Flow Graph ===\n";
    for (const auto& blk : blocks) {
        std::cout << "Block " << blk.id
                  << " | instrs [";
        for (int i : blk.instrIdx) std::cout << i << " ";
        std::cout << "]\n";
        std::cout << "  Succs: ";
        for (int s : blk.successors)   std::cout << s << " ";
        std::cout << "\n  Preds: ";
        for (int p : blk.predecessors) std::cout << p << " ";
        std::cout << "\n";
    }
}