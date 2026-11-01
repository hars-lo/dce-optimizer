#pragma once
#include <string>
#include <vector>

//Instruction opcodes 
enum OpCode {
    OP_ASSIGN,   // result = arg1
    OP_ADD,      // result = arg1 + arg2
    OP_SUB,      // result = arg1 - arg2
    OP_MUL,      // result = arg1 * arg2
    OP_DIV,      // result = arg1 / arg2
    OP_EQ,       // result = (arg1 == arg2)
    OP_NEQ,      // result = (arg1 != arg2)   
    OP_LT,       // result = (arg1 <  arg2)   
    OP_GT,       // result = (arg1 >  arg2)   
    OP_LE,       // result = (arg1 <= arg2)   
    OP_GE,       // result = (arg1 >= arg2)   
    OP_LABEL,    // LABEL:
    OP_JUMP,     // goto LABEL
    OP_JUMPIF,   // if arg1 goto LABEL  (result = label)
    OP_JUMPIFNOT,// if !arg1 goto LABEL
    OP_RETURN,   // return arg1
    OP_PARAM,    // param arg1  (for function calls, bonus)
    OP_CALL,     // result = call arg1 arg2(#params)
    OP_NOP       // no-op 
};

struct Instruction {
    OpCode      op;
    std::string result;  // LHS / destination / label name
    std::string arg1;    // first operand
    std::string arg2;    // second operand (empty if unused)
    int         lineno;  // source line (for diagnostics)
    bool        dead;    // marked dead by DCE pass

    // Convenience constructor
    Instruction(OpCode op,
                std::string res  = "",
                std::string a1   = "",
                std::string a2   = "",
                int         line = 0)
        : op(op), result(std::move(res)),
          arg1(std::move(a1)), arg2(std::move(a2)),
          lineno(line), dead(false) {}
};

using TAC = std::vector<Instruction>;


class TACBuilder {
public:
    TAC instructions;

    int tempCount = 0;

    std::string newTemp() {
        return "t" + std::to_string(tempCount++);
    }


    int emit(OpCode op,
             const std::string& res  = "",
             const std::string& arg1 = "",
             const std::string& arg2 = "") {
        instructions.emplace_back(op, res, arg1, arg2);
        return (int)instructions.size() - 1;
    }

    void patch(int instrIdx, const std::string& label) {
        instructions[instrIdx].result = label;
    }

    int nextIndex() const { return (int)instructions.size(); }

    void print() const;
};