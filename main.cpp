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
    if (errno == ERANGE) throw runtime_error("read: invalid number");
    return NUM_VAL(x);
}

lval lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) return lval_read_num(t);
    if (strstr(t->tag, "bool")) {
        return t->contents == string("#t")
            ? BOOL_VAL(true) : BOOL_VAL(false);
    }
    if (strstr(t->tag, "symbol")) return SYM_VAL(t->contents);
    if (strstr(t->tag, "qexpr")) {
        vector<lval> x;
        x.push_back(SYM_VAL("quote"));
        x.push_back(lval_read(t->children[1]));
        return SXP_VAL(x);
    }

    vector<lval> x;
    for (int i = 1; i < t->children_num - 1; i++) {
        // if (string(t->children[i]->contents) == "(") continue;
        // if (string(t->children[i]->contents) == ")") continue;
        // if (string(t->children[i]->tag) == "regex") continue;
        x.push_back(lval_read(t->children[i]));
    }
    return SXP_VAL(x);
}

int main(int argc, char *argv[]) {
    cout << "Lispy Version 0.0.0.0.1\n";
    cout << "Press Ctrl+d to Exit\n\n";

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Bool   = mpc_new("bool");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Qexpr  = mpc_new("qexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "number : /[+-]?[0-9]+([.][0-9]+)?/ ;"
        "bool   : \"#t\" | \"#f\";"
        "symbol : /[A-Za-z+\\-*\\/_<=>?!]"
                  "[A-Za-z+\\-*\\/_<=>?!0-9]*/ ;"
        "sexpr  : '(' <expr>* ')' ;"
        "qexpr  : '\\'' <expr> ;"
        "expr   : <number> | <bool> | <symbol> | <sexpr> | <qexpr> ;"
        "lispy  : /^/ <expr> /$/ ;",
        Number, Bool, Symbol, Sexpr, Qexpr, Expr, Lispy);

    lenv env;
    lenv_add_builtins(env);
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
            try {
                lval v = lval_read(t->children[1]);
                cout << v << "\n";
                lval result = v.eval(env);
                cout << result << "\n";
            } catch (exception& e) {
                cout << e.what() << "\n";
            }
            mpc_ast_delete(t);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(7, Number, Bool, Symbol, Sexpr, Qexpr, Expr, Lispy);
}