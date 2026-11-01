#pragma once

#include "cfg.h"

void eliminateDeadCode(CFG& cfg, TAC& tac);
void printOptimizedTAC(const TAC& tac);
