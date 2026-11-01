#include "constant_propagation.h"

#include <cctype>
#include <string>
#include <unordered_map>

//Simple constant propagation within a single basic block
// If we see:  t0 = 5
//             t1 = t0 + 3
// We replace: t1 = 5 + 3  →  t1 = 8  (if both args are constants)

void constantPropagation(TAC& tac) {
    std::unordered_map<std::string, std::string> constMap;

    auto isConst = [&](const std::string& s) -> bool {
        if (s.empty()) return false;
        if (std::isdigit(s[0]) || (s[0] == '-' && s.size() > 1))
            return true;
        return constMap.count(s) > 0;
    };

    auto getValue = [&](const std::string& s) -> std::string {
        if (std::isdigit(s[0])) return s;
        auto it = constMap.find(s);
        return it != constMap.end() ? it->second : s;
    };

    auto foldBinary = [](OpCode op,
                         const std::string& a,
                         const std::string& b) -> std::string {
        try {
            double va = std::stod(a);
            double vb = std::stod(b);
            double result = 0;
            switch (op) {
                case OP_ADD: result = va + vb; break;
                case OP_SUB: result = va - vb; break;
                case OP_MUL: result = va * vb; break;
                case OP_DIV:
                    if (vb == 0) return "";   // avoid div by zero
                    result = va / vb; break;
                case OP_EQ:  result = (va == vb) ? 1 : 0; break;
                case OP_NEQ: result = (va != vb) ? 1 : 0; break;
                case OP_LT:  result = (va <  vb) ? 1 : 0; break;
                case OP_GT:  result = (va >  vb) ? 1 : 0; break;
                case OP_LE:  result = (va <= vb) ? 1 : 0; break;
                case OP_GE:  result = (va >= vb) ? 1 : 0; break;
                default: return "";
            }
            // Return as integer string if no fractional part
            if (result == (int)result)
                return std::to_string((int)result);
            return std::to_string(result);
        } catch (...) { return ""; }
    };

    for (auto& ins : tac) {
        if (ins.dead) continue;


        if (ins.op == OP_LABEL) {
            constMap.clear();
            continue;
        }


        if (constMap.count(ins.arg1)) ins.arg1 = constMap[ins.arg1];
        if (constMap.count(ins.arg2)) ins.arg2 = constMap[ins.arg2];


        if ((ins.op==OP_ADD||ins.op==OP_SUB||ins.op==OP_MUL||ins.op==OP_DIV||
             ins.op==OP_EQ ||ins.op==OP_NEQ||ins.op==OP_LT ||ins.op==OP_GT||
             ins.op==OP_LE ||ins.op==OP_GE) &&
            isConst(ins.arg1) && isConst(ins.arg2)) {

            std::string folded = foldBinary(ins.op,
                                            getValue(ins.arg1),
                                            getValue(ins.arg2));
            if (!folded.empty()) {
                ins.op   = OP_ASSIGN;
                ins.arg1 = folded;
                ins.arg2 = "";
                constMap[ins.result] = folded;
            }
        } else if (ins.op == OP_ASSIGN && isConst(ins.arg1)) {
            constMap[ins.result] = getValue(ins.arg1);
        } else {
            constMap.erase(ins.result);
        }
    }
}