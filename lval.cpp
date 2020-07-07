#include "lval.hpp"
#include <cstring>
#include <stdexcept>
using namespace std;

lval::lval(LvalType type, double x) : type(type), as {.num = x} {}

lval::lval(LvalType type, const string& m) : type(type), as {.sym = new string(m)} {}

lval::lval(LvalType type, const vector<lval>& v) : type(type), as {.vec = new vector<lval> (v)} {}

lval lval::eval_sexpr() const {
    if (size() == 0) {
        throw runtime_error("s-expr: empty");
    }
    if (at(0).is_quote()) {
        if (size() != 2) {
            throw runtime_error("quote: wrong # of arguments");
        }
        return at(1);
    }

    lval f = at(0).eval();
    if (f.type != LVAL_SYM) {
        throw runtime_error("s-expr: invalid procedure");
    }

    vector<lval> res;
    for (int i = 1; i < size(); i++) {
        res.push_back(at(i).eval());
    }

    lval tail = SXP_VAL(res);
    return tail.builtin(AS_SYM(f));
}

lval lval::eval() const {
    if (type == LVAL_SXP) return eval_sexpr();
    return *this;
}

bool lval::is_quote() const {
    return type == LVAL_SYM && *as.sym == "quote";
}

#define LASSERT(cond, err) \
    if (!(cond)) { throw runtime_error(err); }

lval lval::builtin(const string& func) {
    if (func == "head") return builtin_head();
    if (func == "tail") return builtin_tail();
    if (func == "list") return builtin_list();
    if (func == "eval") return builtin_eval();
    if (func == "join") return builtin_join();
    if (strstr("+-*/", func.c_str())) return builtin_op(func);
    throw runtime_error(func + ": unknown function");
}

lval lval::builtin_op(const string& op) {
    for (const auto& v : *as.vec) {
        LASSERT(
            IS_NUM(v),
            "builtin_op: non-numeric argument"
        );
    }

    if (op == "-" && size() == 1) {
        return NUM_VAL(-AS_NUM(at(0)));
    }

    LASSERT(
        size() >= 2,
        "builtin_op: wrong # of arguments"
    );

    double x = AS_NUM(at(0));
    for (int i = 1; i < size(); i++) {
        double y = AS_NUM(at(i));
        if (op == "+") x += y;
        if (op == "-") x -= y;
        if (op == "*") x *= y;
        if (op == "/") {
            if (y == 0) {
                throw runtime_error("builtin_op: division by 0");
            }
            x /= y;
        }
    }

    return NUM_VAL(x);
}

lval lval::builtin_head() {
    LASSERT(
        size() == 1,
        "head: wrong # of arg(s)"
    );
    LASSERT(
        at(0).type == LVAL_SXP,
        "head: wrong arg type"
    );
    LASSERT(
        at(0).size() > 0,
        "head: empty arg"
    );
    return at(0).at(0);
}

lval lval::builtin_tail() {
    LASSERT(
        size() == 1,
        "tail: wrong # of arg(s)"
    );
    LASSERT(
        at(0).type == LVAL_SXP,
        "tail: wrong arg type"
    );
    LASSERT(
        at(0).size() > 0,
        "tail: empty arg"
    );
    return at(0).tail();
}

lval lval::builtin_join() {
    vector<lval> x;
    for (const auto& v : *as.vec) {
        LASSERT(
            v.type == LVAL_SXP,
            "join: wrong arg type"
        );
        for (const auto& w : *v.as.vec) { x.emplace_back(w); }
    }
    return SXP_VAL(x);
}

lval lval::builtin_list() {
    return *this;
}

lval lval::builtin_eval() {
    LASSERT(
        size() == 1,
        "eval: wrong # of arg(s)"
    );
    return at(0).eval();
}

ostream& lval::print(ostream& out) const {
    switch (type) {
    case LVAL_NUM: out << as.num; break;
    case LVAL_SYM: out << *as.sym; break;
    case LVAL_SXP:
        out << "(";
        for (int i = 0; i < size(); i++) {
            out << at(i);
            if (i < size() - 1) out << " ";
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