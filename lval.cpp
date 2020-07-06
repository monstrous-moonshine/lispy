#include "lval.hpp"

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