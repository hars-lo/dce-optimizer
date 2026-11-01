#include <iostream>
#include <fstream>
#include <cstdio>
#include "tac.h"
#include "cfg.h"
#include "liveness.h"
#include "dce.h"
#include "unreachable.h"
#include "constant_propagation.h"

extern TACBuilder builder;   
extern FILE* yyin;
extern int yyparse();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>\n";
        return 1;
    }

    //Phase 1: Parse and generate TAC
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror("fopen"); return 1; }

    std::cout << "=== Parsing ===\n";
    if (yyparse() != 0) {
        std::cerr << "Parse failed.\n"; return 1;
    }
    fclose(yyin);

    TAC& tac = builder.instructions;
    builder.print();

    //Phase 2: Unreachable code elimination 
    std::cout << "\n=== Unreachable Code Elimination ===\n";
    // Build CFG first for block-level reachability
    CFG cfg;
    cfg.build(tac);
    cfg.print();
    markReachable(cfg, tac);

    //Phase 3: Constant Propagation
    std::cout << "\n=== Constant Propagation ===\n";
    constantPropagation(tac);
    builder.print();   // show state after CP

    //Phase 4: Rebuild CFG
    cfg = CFG{};
    cfg.build(tac);

    //Phase 5: Liveness Analysis
    std::cout << "\n=== Liveness Analysis ===\n";
    computeUseDef(cfg, tac);
    computeLiveness(cfg);
    printLiveness(cfg);

    //Phase 6: Dead Code Elimination
    std::cout << "\n=== Dead Code Elimination ===\n";
    eliminateDeadCode(cfg, tac);

    //Phase 7: Output
    printOptimizedTAC(tac);

    return 0;
}