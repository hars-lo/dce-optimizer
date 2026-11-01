#pragma once

#include "cfg.h"

void computeUseDef(CFG& cfg, const TAC& tac);
void computeLiveness(CFG& cfg);
void printLiveness(const CFG& cfg);
