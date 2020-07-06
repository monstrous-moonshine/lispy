#include "lval.hpp"
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
    lval x = (*as.cell)[i];
    as.cell->erase(as.cell->begin() + i);
    return x;
}

lval lval::take(int i) {
    lval x = (*as.cell)[i];
    delete as.cell;
    return x;
}

lval lval::get(int i) {
    return (*as.cell)[i];
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

lval lval::eval_sexpr() {
    if (get(0).is_quote()) {
        if (count() != 2) {
            return ERR_VAL("quote: wrong # of args");
        }
        return get(1);
    }

    for (auto& v : *as.cell) {
        v = v.eval();
        if (v.type == LVAL_ERR) return v;
    }

    if (count() == 0) return *this;
    if (count() == 1) return get(0);

    lval f = pop(0);
    if (f.type != LVAL_SYM) {
        return ERR_VAL("s-expression does not start with symbol");
    }

    return builtin_op(AS_SYM(f));
}

lval lval::eval() {
    if (type == LVAL_SEXPR) return eval_sexpr();
    return *this;
}

bool lval::is_quote() {
    return type == LVAL_SYM && *as.sym == "quote";
}

ostream& lval::print(ostream& out) const {
    switch (type) {
    case LVAL_NUM: out << as.num; break;
    case LVAL_ERR: out << "Error: " << *as.err; break;
    case LVAL_SYM: out << *as.sym; break;
    case LVAL_SEXPR:
        out << "(";
        for (int i = 0; i < as.cell->size(); i++) {
            out << (*as.cell)[i];
            if (i < as.cell->size() - 1) out << " ";
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