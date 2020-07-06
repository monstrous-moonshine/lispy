#include <iostream>
#include "mpc.h"
#include "lval.hpp"
using namespace std;

extern "C" {
char* rl_gets(const char* prompt);
}

lval lval_read_num(mpc_ast_t* t) {
    errno = 0;
    double x = strtod(t->contents, NULL);
    return errno != ERANGE ?
        NUM_VAL(x) : ERR_VAL("invalid number");
}

lval lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) return lval_read_num(t);
    if (strstr(t->tag, "symbol")) return SYM_VAL(t->contents);

    lval x = SXP_VAL();
    for (int i = 0; i < t->children_num; i++) {
        if (string(t->children[i]->contents) == "(") continue;
        if (string(t->children[i]->contents) == ")") continue;
        if (string(t->children[i]->tag) == "regex") continue;
        x.as.cell->push_back(lval_read(t->children[i]));
    }

    return x;
}

lval eval_op(char* op_, lval x_, lval y_) {
    if (x_.type == LVAL_ERR) return x_;
    if (y_.type == LVAL_ERR) return y_;
    double x = x_.as.num;
    double y = y_.as.num;
    string op(op_);
    if (op == "+") { return NUM_VAL(x + y); }
    if (op == "-") { return NUM_VAL(x - y); }
    if (op == "*") { return NUM_VAL(x * y); }
    if (op == "/") {
        return y == 0
            ? ERR_VAL(0)
            : NUM_VAL(x / y);
    }
    // unreachable
    return ERR_VAL(0);
}

lval eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        errno = 0;
        double x = strtod(t->contents, NULL);
        return errno != ERANGE ? NUM_VAL(x) : ERR_VAL(0);
    }

    char* op = t->children[1]->contents;
    lval x = eval(t->children[2]);
    for (int i = 3; strstr(t->children[i]->tag, "expr"); i++) {
        lval y = eval(t->children[i]);
        x = eval_op(op, x, y);
    }

    return x;
}

int main(int argc, char *argv[]) {
    cout << "Lispy Version 0.0.0.0.1\n";
    cout << "Press Ctrl+d to Exit\n\n";

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "number : /[+-]?[0-9]+([.][0-9]+)?/ ;"
        "symbol : '+' | '-' | '*' | '/' ;"
        "sexpr  : '(' <expr>* ')' ;"
        "expr   : <number> | <symbol> | <sexpr> ;"
        "lispy  : /^/ <expr> /$/ ;",
        Number, Symbol, Sexpr, Expr, Lispy);

    while (true) {
        char* input = rl_gets("lispy> ");
        if (!input) {
            cout << "\n";
            break;
        }
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            mpc_ast_t* output = static_cast<mpc_ast_t*>(r.output);
            mpc_ast_print(output);
            //lval result = eval(output->children[1]);
            //cout << result << "\n";
            lval v = lval_read(output->children[1]);
            cout << v << "\n";
            mpc_ast_delete(output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
}