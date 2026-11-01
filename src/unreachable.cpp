#include "unreachable.h"
#include <iostream>
#include <stack>
#include <vector>

void markReachable(CFG& cfg, TAC& tac) {
    int n = (int)cfg.blocks.size();
    std::vector<bool> visited(n, false);

    // DFS stack
    std::stack<int> stk;
    stk.push(0);   // entry block

    while (!stk.empty()) {
        int b = stk.top(); stk.pop();
        if (visited[b]) continue;
        visited[b] = true;
        for (int succ : cfg.blocks[b].successors) {
            if (!visited[succ])
                stk.push(succ);
        }
    }

    int killed = 0;
    for (int b = 0; b < n; b++) {
        if (!visited[b]) {
            std::cout << "  Block " << b << " is UNREACHABLE\n";
            for (int idx : cfg.blocks[b].instrIdx) {
                if (!tac[idx].dead) {
                    tac[idx].dead = true;
                    killed++;
                }
            }
        }
    }

    for (auto& blk : cfg.blocks) {
        bool seenReturn = false;
        for (int idx : blk.instrIdx) {
            if (seenReturn && !tac[idx].dead) {
                tac[idx].dead = true;
                killed++;
            }
            if (tac[idx].op == OP_RETURN)
                seenReturn = true;
        }
    }

    std::cout << "Unreachable code elimination killed "
              << killed << " instruction(s).\n";
}