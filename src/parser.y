%{
/* parser.y — Bison parser with TAC generation */
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include "tac.h"

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

// Global TAC builder (accessible from actions)
TACBuilder builder;

// Portable replacement for POSIX dup_cstr()
static char* dup_cstr(const char* s) {
    if (s == nullptr) return nullptr;

    std::size_t n = std::strlen(s) + 1;
    char* copy = static_cast<char*>(std::malloc(n));

    if (copy != nullptr) {
        std::memcpy(copy, s, n);
    }

    return copy;
}

// Label counter
static int labelCount = 0;
std::string newLabel() {
    return "L" + std::to_string(labelCount++);
}

// ── Backpatching contexts for nested if/else and while ───────────────────
// Labels are generated eagerly (we know the name up front), so these stacks
// just carry the label names between the mid-rule actions of one (possibly
// nested) if/while construct. LIFO order matches bison's reduction order
// for properly nested constructs, including nested if-inside-if and
// if-inside-while.
static std::vector<std::string> ifFalseStack;   // label to jump to on false
static std::vector<std::string> ifEndStack;     // label after an else-body
static std::vector<std::string> whileStartStack;// label at loop top
static std::vector<std::string> whileEndStack;  // label after loop
%}

/* ── Value types carried by grammar symbols ─────────────────────────── */
%union {
    int   ival;
    float fval;
    char* sval;
}

/* ── Token declarations ──────────────────────────────────────────────── */
%token <sval> ID
%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token IF ELSE RETURN WHILE TINT TFLOAT
%token PLUS MINUS STAR SLASH ASSIGN
%token EQ NEQ LT GT LE GE
%token LPAREN RPAREN LBRACE RBRACE SEMI COMMA

/* ── Operator precedence (lowest → highest) ──────────────────────────── */
%right ASSIGN
%left  EQ NEQ
%left  LT GT LE GE
%left  PLUS MINUS
%left  STAR SLASH
%right UMINUS

/* ── Non-terminal types ──────────────────────────────────────────────── */
%type <sval> expr stmt stmtlist

%%

/* ================================================================
   GRAMMAR RULES + TAC GENERATION ACTIONS
   ================================================================ */

program
    : stmtlist          { /* root — TAC is fully built */ }
    ;

stmtlist
    : stmtlist stmt     { $$ = nullptr; }
    | stmt              { $$ = nullptr; }
    ;

stmt
    /* ── Variable assignment: id = expr; ──────────────────────── */
    : ID ASSIGN expr SEMI {
        // Emit:  id = expr_result
        builder.emit(OP_ASSIGN, $1, $3);
        free($1); free($3);
        $$ = nullptr;
    }

    /* ── return expr; ─────────────────────────────────────────── */
    | RETURN expr SEMI {
        builder.emit(OP_RETURN, "", $2);
        free($2);
        $$ = nullptr;
    }

    /* ── if (expr) { stmtlist } [else { stmtlist }] ──────────────
       Labels are generated and emitted eagerly, so no patch() call
       is needed — we always know the target label's name at the
       point we emit the jump; we just emit the OP_LABEL itself
       later, once we reach the right spot in the instruction stream. */
    | IF LPAREN expr RPAREN {
        std::string falseLabel = newLabel();
        builder.emit(OP_JUMPIFNOT, falseLabel, $3);
        free($3);
        ifFalseStack.push_back(falseLabel);
    } LBRACE stmtlist RBRACE elsepart {
        $$ = nullptr;
    }

    /* ── while (expr) { stmtlist } ───────────────────────────── */
    | WHILE {
        std::string startLabel = newLabel();
        builder.emit(OP_LABEL, startLabel);
        whileStartStack.push_back(startLabel);
    } LPAREN expr RPAREN {
        std::string endLabel = newLabel();
        builder.emit(OP_JUMPIFNOT, endLabel, $4);
        free($4);
        whileEndStack.push_back(endLabel);
    } LBRACE stmtlist RBRACE {
        std::string startLabel = whileStartStack.back(); whileStartStack.pop_back();
        builder.emit(OP_JUMP, startLabel);
        std::string endLabel = whileEndStack.back(); whileEndStack.pop_back();
        builder.emit(OP_LABEL, endLabel);
        $$ = nullptr;
    }

    /* ── { stmtlist } ─────────────────────────────────────────── */
    | LBRACE stmtlist RBRACE { $$ = nullptr; }

    /* ── Type declarations: int x; ────────────────────────────── */
    | TINT ID SEMI   { free($2); $$ = nullptr; }
    | TFLOAT ID SEMI { free($2); $$ = nullptr; }
    ;

/* ── Optional else-branch for 'if'. Placed at the end of the IF rule so
   that bison's default shift resolves the classic dangling-else the
   conventional way: an 'else' always binds to the nearest unmatched 'if'. */
elsepart
    : {
        std::string falseLabel = ifFalseStack.back(); ifFalseStack.pop_back();
        builder.emit(OP_LABEL, falseLabel);
    }
    | ELSE LBRACE {
        std::string falseLabel = ifFalseStack.back(); ifFalseStack.pop_back();
        std::string endLabel = newLabel();
        builder.emit(OP_JUMP, endLabel);        // skip else-body if if-body ran
        builder.emit(OP_LABEL, falseLabel);      // else-body starts here
        ifEndStack.push_back(endLabel);
    } stmtlist RBRACE {
        std::string endLabel = ifEndStack.back(); ifEndStack.pop_back();
        builder.emit(OP_LABEL, endLabel);
    }
    ;

/* ── Expressions — each returns the name of the temp/var holding value ── */
expr
    /* Arithmetic */
    : expr PLUS expr {
        std::string t = builder.newTemp();
        builder.emit(OP_ADD, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr MINUS expr {
        std::string t = builder.newTemp();
        builder.emit(OP_SUB, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr STAR expr {
        std::string t = builder.newTemp();
        builder.emit(OP_MUL, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr SLASH expr {
        std::string t = builder.newTemp();
        builder.emit(OP_DIV, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    /* Unary minus */
    | MINUS expr %prec UMINUS {
        std::string t = builder.newTemp();
        builder.emit(OP_SUB, t, "0", $2);
        free($2);
        $$ = dup_cstr(t.c_str());
    }
    /* Comparisons — each produces a temp holding 1 (true) or 0 (false) */
    | expr EQ expr {
        std::string t = builder.newTemp();
        builder.emit(OP_EQ, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr NEQ expr {
        std::string t = builder.newTemp();
        builder.emit(OP_NEQ, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr LT expr {
        std::string t = builder.newTemp();
        builder.emit(OP_LT, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr GT expr {
        std::string t = builder.newTemp();
        builder.emit(OP_GT, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr LE expr {
        std::string t = builder.newTemp();
        builder.emit(OP_LE, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    | expr GE expr {
        std::string t = builder.newTemp();
        builder.emit(OP_GE, t, $1, $3);
        free($1); free($3);
        $$ = dup_cstr(t.c_str());
    }
    /* Parentheses */
    | LPAREN expr RPAREN { $$ = $2; }
    /* Literals → emit ASSIGN to a temp */
    | INT_LIT {
        std::string t = builder.newTemp();
        builder.emit(OP_ASSIGN, t, std::to_string($1));
        $$ = dup_cstr(t.c_str());
    }
    | FLOAT_LIT {
        std::string t = builder.newTemp();
        builder.emit(OP_ASSIGN, t, std::to_string($1));
        $$ = dup_cstr(t.c_str());
    }
    /* Variable reference */
    | ID {
        $$ = $1;   // just pass the name — no new temp needed
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}



