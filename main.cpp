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
    if (strstr(t->tag, "qexpr")) {
        lval x = SXP_VAL();
        x.push_back(SYM_VAL("quote"));
        x.push_back(lval_read(t->children[1]));
        return x;
    }

    lval x = SXP_VAL();
    for (int i = 1; i < t->children_num - 1; i++) {
        // if (string(t->children[i]->contents) == "(") continue;
        // if (string(t->children[i]->contents) == ")") continue;
        // if (string(t->children[i]->tag) == "regex") continue;
        x.push_back(lval_read(t->children[i]));
    }

    return x;
}

int main(int argc, char *argv[]) {
    cout << "Lispy Version 0.0.0.0.1\n";
    cout << "Press Ctrl+d to Exit\n\n";

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Qexpr  = mpc_new("qexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "number : /[+-]?[0-9]+([.][0-9]+)?/ ;"
        "symbol : '+' | '-' | '*' | '/' | \"quote\""
        "       | \"list\" | \"head\" | \"tail\" | \"join\""
        "       | \"eval\" ;"
        "sexpr  : '(' <expr>* ')' ;"
        "qexpr  : '\\'' <expr> ;"
        "expr   : <number> | <symbol> | <sexpr> | <qexpr> ;"
        "lispy  : /^/ <expr> /$/ ;",
        Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    while (true) {
        char* input = rl_gets("lispy> ");
        if (!input) {
            cout << "\n";
            break;
        }
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            mpc_ast_t* t = static_cast<mpc_ast_t*>(r.output);
            mpc_ast_print(t);
            lval v = lval_read(t->children[1]);
            cout << v << "\n";
            lval result = v.eval();
            cout << result << "\n";
            mpc_ast_delete(t);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
}