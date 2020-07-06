#include "lval.hpp"
#include <cstring>
using namespace std;

lval::lval(LvalType type, double x) : type(type) {
    as.num = x;
}

lval::lval(LvalType type, const string& m) : type(type) {
    switch (type) {
    case LVAL_ERR:
        as.err = new string(m); break;
    case LVAL_SYM:
        as.sym = new string(m); break;
    }
}

lval::lval(LvalType type) : type(type) {
    as.cell = new vector<lval>();
}

lval::~lval() {}

lval lval::pop(int i) {
    // TODO: bounds check?
    lval x = get(i);
    as.cell->erase(as.cell->begin() + i);
    return x;
}

lval lval::take(int i) {
    // TODO: bounds check?
    lval x = get(i);
    delete as.cell;
    return x;
}

lval lval::eval_sexpr() {
    if (get(0).is_quote()) {
        if (count() != 2) {
            return ERR_VAL("quote: wrong # of arg(s)");
        }
        return get(1);
    }

    for (auto& v : *as.cell) {
        v = v.eval();
        if (v.type == LVAL_ERR) return v;
    }

    // TODO: review these 2 lines
    if (count() == 0) return *this;
    if (count() == 1) return get(0);

    lval f = pop(0);
    if (f.type != LVAL_SYM) {
        return ERR_VAL("s-expression does not start with symbol");
    }

    return builtin(AS_SYM(f));
}

lval lval::eval() {
    if (type == LVAL_SEXPR) return eval_sexpr();
    return *this;
}

bool lval::is_quote() const {
    return type == LVAL_SYM && *as.sym == "quote";
}

lval lval::builtin(const string& func) {
    if (func == "head") return builtin_head();
    if (func == "tail") return builtin_tail();
    if (func == "list") return builtin_list();
    if (func == "eval") return builtin_eval();
    if (func == "join") return builtin_join();
    if (strstr("+-*/", func.c_str())) return builtin_op(func);
    return ERR_VAL(func + ": unknown function");
}

lval lval::builtin_op(const string& op) {
    lval x = get(0);
    if (op == "-" && count() == 1) {
        AS_NUM(x) = -AS_NUM(x);
    }

    for (int i = 1; i < count(); i++) {
        lval y = get(i);
        if (op == "+") AS_NUM(x) += AS_NUM(y);
        if (op == "-") AS_NUM(x) -= AS_NUM(y);
        if (op == "*") AS_NUM(x) *= AS_NUM(y);
        if (op == "/") {
            if (AS_NUM(y) == 0) {
                return ERR_VAL("division by 0");
            }
            AS_NUM(x) /= AS_NUM(y);
        }
    }

    return x;
}

#define LASSERT(cond, err) \
    if (!(cond)) { return ERR_VAL(err); }

lval lval::builtin_head() {
    LASSERT(
        count() == 1,
        "head: wrong # of arg(s)"
    );
    LASSERT(
        get(0).type == LVAL_SEXPR,
        "head: wrong arg type"
    );
    LASSERT(
        get(0).count() > 0,
        "head: empty arg"
    );
    return get(0).get(0);
}

lval lval::builtin_tail() {
    LASSERT(
        count() == 1,
        "tail: wrong # of arg(s)"
    );
    LASSERT(
        get(0).type == LVAL_SEXPR,
        "tail: wrong arg type"
    );
    LASSERT(
        get(0).count() > 0,
        "tail: empty arg"
    );
    get(0).pop(0);
    return get(0);
}

lval lval::builtin_join() {
    lval x = SXP_VAL();
    for (const auto& v : *as.cell) {
        LASSERT(
            v.type == LVAL_SEXPR,
            "join: wrong arg type"
        );
        AS_VEC(x).insert(
            AS_VEC(x).end(), 
            AS_VEC(v).begin(), AS_VEC(v).end()
        );
    }
    return x;
}

lval lval::builtin_list() {
    return *this;
}

lval lval::builtin_eval() {
    LASSERT(
        count() == 1,
        "eval: wrong # of arg(s)"
    );
    return get(0).eval();
}

ostream& lval::print(ostream& out) const {
    switch (type) {
    case LVAL_NUM: out << as.num; break;
    case LVAL_ERR: out << "Error: " << *as.err; break;
    case LVAL_SYM: out << *as.sym; break;
    case LVAL_SEXPR:
        out << "(";
        for (int i = 0; i < count(); i++) {
            out << get(i);
            if (i < count() - 1) out << " ";
        }
        out << ")";
    }
    return out;
}

ostream& lval::println(ostream& out) const {
    print(out);
    out << "\n";
    return out;
}

ostream& operator<<(ostream& out, const lval& v) {
    return v.print(out);
}