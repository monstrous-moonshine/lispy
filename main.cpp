#include <iostream>
#include "mpc.h"
using namespace std;

extern "C" {
char* rl_gets(const char* prompt);
}

enum LvalType {
    LVAL_NUM, LVAL_ERR
};

enum LerrType {
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

struct lval {
    LvalType type;
    double num;
    LerrType err;
    lval(double x) {
        type = LVAL_NUM;
        num = x;
    }
    lval(LerrType x) {
        type = LVAL_ERR;
        err = x;
    }
    ostream& print(ostream& out) const {
        switch (type) {
        case LVAL_NUM: out << num; break;
        case LVAL_ERR:
            out << "Error: ";
            switch (err) {
            case LERR_DIV_ZERO:
                out << "Division By Zero!"; break;
            case LERR_BAD_OP:
                out << "Invalid Operator!"; break;
            case LERR_BAD_NUM:
                out << "Invalid Number!"; break;
            }
        }
        return out;
    }
    ostream& println(ostream& out) const {
        print(out);
        out << "\n";
        return out;
    }
    friend ostream& operator<<(ostream& out, const lval& v) {
        return v.print(out);
    }
};

lval eval_op(char* op_, lval x_, lval y_) {
    if (x_.type == LVAL_ERR) return x_;
    if (y_.type == LVAL_ERR) return y_;
    double x = x_.num;
    double y = y_.num;
    string op(op_);
    if (op == "+") { return x + y; }
    if (op == "-") { return x - y; }
    if (op == "*") { return x * y; }
    if (op == "/") {
        return y == 0
            ? lval(LERR_DIV_ZERO)
            : x / y;
    }
    // unreachable
    return 0;
}

lval eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        errno = 0;
        double x = strtod(t->contents, NULL);
        return errno != ERANGE ? lval(x) : lval(LERR_BAD_NUM);
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

    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    mpca_lang(
        MPCA_LANG_DEFAULT,
        "number   : /[+-]?[0-9]+([.][0-9]+)?/ ;"
        "operator : '+' | '-' | '*' | '/' ;"
        "expr     : <number> | '(' <operator> <expr>+ ')' ;"
        "lispy    : /^/ <expr> /$/ ;",
        Number, Operator, Expr, Lispy);

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
            lval result = eval(output->children[1]);
            cout << result << "\n";
            mpc_ast_delete(output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);
}