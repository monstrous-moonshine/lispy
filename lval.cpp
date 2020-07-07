#include "lval.hpp"
#include <cstring>
#include <stdexcept>
using namespace std;

lval::lval(LvalType type, double x) : type(type), as {.num = x} {}

lval::lval(LvalType type, const string& m) : type(type), as {.sym = new string(m)} {}

lval::lval(LvalType type, lbuiltin fun): type(type), as {.fun = fun} {}

lval::lval(LvalType type, const vector<lval>& v) : type(type), as {.vec = new vector<lval> (v)} {}

#define LASSERT(cond, err) \
    if (!(cond)) { throw runtime_error(err); }

lval lval::eval_sexpr(lenv& e) const {
    LASSERT(
        size() > 0,
        "s-expr: empty"
    );
    if (at(0).is_quote()) {
        LASSERT(
            size() == 2,
            "quote: wrong # of arguments"
        );
        return at(1);
    }
    if (at(0).is_def()) {
        LASSERT(
            size() == 3,
            "define: wrong # of arguments"
        );
        LASSERT(
            IS_SYM(at(1)),
            "define: target must be symbol"
        );
        e.emplace(AS_SYM(at(1)), at(2));
        return at(2);
    }

    lval f = at(0).eval(e);
    LASSERT(
        IS_FUN(f),
        "s-expr: invalid procedure"
    );

    vector<lval> res;
    for (int i = 1; i < size(); i++) {
        res.push_back(at(i).eval(e));
    }
    lval tail = SXP_VAL(res);
    return AS_FUN(f)(tail, e);
}

lval lval::eval(lenv& e) const {
    if (type == LVAL_SYM) {
        LASSERT(
            e.count(*as.sym) > 0,
            "eval: undefined variable '" + *as.sym + "'"
        );
        return e.at(*as.sym);
    }
    if (type == LVAL_SXP) return eval_sexpr(e);
    return *this;
}

bool lval::is_quote() const {
    return type == LVAL_SYM && *as.sym == "quote";
}

bool lval::is_def() const {
    return type == LVAL_SYM && *as.sym == "define";
}

lval lval::builtin_op(const string& op, lenv& e) {
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

lval builtin_add(lval v, lenv& e) {
    return v.builtin_op("+", e);
}

lval builtin_sub(lval v, lenv& e) {
    return v.builtin_op("-", e);
}

lval builtin_mul(lval v, lenv& e) {
    return v.builtin_op("*", e);
}

lval builtin_div(lval v, lenv& e) {
    return v.builtin_op("/", e);
}

lval builtin_head(lval v, lenv& e) {
    LASSERT(
        v.size() == 1,
        "head: wrong # of arg(s)"
    );
    LASSERT(
        v.at(0).type == LVAL_SXP,
        "head: wrong arg type"
    );
    LASSERT(
        v.at(0).size() > 0,
        "head: empty arg"
    );
    return v.at(0).at(0);
}

lval builtin_tail(lval v, lenv& e) {
    LASSERT(
        v.size() == 1,
        "tail: wrong # of arg(s)"
    );
    LASSERT(
        v.at(0).type == LVAL_SXP,
        "tail: wrong arg type"
    );
    LASSERT(
        v.at(0).size() > 0,
        "tail: empty arg"
    );
    return v.at(0).tail();
}

lval builtin_join(lval v, lenv& e) {
    vector<lval> x;
    for (const auto& v : *v.as.vec) {
        LASSERT(
            v.type == LVAL_SXP,
            "join: wrong arg type"
        );
        for (const auto& w : *v.as.vec) { x.emplace_back(w); }
    }
    return SXP_VAL(x);
}

lval builtin_list(lval v, lenv& e) {
    return v;
}

lval builtin_eval(lval v, lenv& e) {
    LASSERT(
        v.size() == 1,
        "eval: wrong # of arg(s)"
    );
    return v.at(0).eval(e);
}

void lenv_add_builtins(lenv& e) {
    e.emplace("+", FUN_VAL(builtin_add));
    e.emplace("-", FUN_VAL(builtin_sub));
    e.emplace("*", FUN_VAL(builtin_mul));
    e.emplace("/", FUN_VAL(builtin_div));
    e.emplace("list", FUN_VAL(builtin_list));
    e.emplace("head", FUN_VAL(builtin_head));
    e.emplace("tail", FUN_VAL(builtin_tail));
    e.emplace("eval", FUN_VAL(builtin_eval));
    e.emplace("join", FUN_VAL(builtin_join));
}

ostream& lval::print(ostream& out) const {
    switch (type) {
    case LVAL_NUM: out << as.num; break;
    case LVAL_SYM: out << *as.sym; break;
    case LVAL_BLT: out << "<fun>"; break;
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