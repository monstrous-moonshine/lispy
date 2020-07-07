#include "lval.hpp"
#include <cstring>
#include <stdexcept>
using namespace std;

lval::lval(LvalType type, double x) : type(type), as {.num = x} {}

lval::lval(LvalType type, const string& m) : type(type), as {.sym = new string(m)} {}

lval::lval(LvalType type, lbuiltin blt): type(type), as {.blt = blt} {}

lval::lval(LvalType type, const lval_fun& fun) : type(type), as {.fun = new lval_fun(fun)} {}

lval::lval(LvalType type, const vector<lval>& v) : type(type), as {.vec = new vector<lval> (v)} {}

#define LASSERT(cond, err) \
    if (!(cond)) { throw runtime_error(err); }

#define LASSERT_NUM(n, err) LASSERT(size() == (n), (err))

lval lval::eval_sexpr(lenv& e) const {
    LASSERT(size() > 0, "s-expr: empty");
    
    // quote
    if (at(0).is_sym("quote")) {
        LASSERT_NUM(2, "quote: wrong # of arguments");
        return at(1);
    }
    
    // define
    if (at(0).is_sym("define")) {
        LASSERT_NUM(3, "define: wrong # of arguments");
        LASSERT(IS_SYM(at(1)), "define: target must be symbol");
        e.emplace(AS_SYM(at(1)), at(2).eval(e));
        return e.at(AS_SYM(at(1)));
    }
    
    // lambda
    if (at(0).is_sym("lambda")) {
        LASSERT_NUM(3, "lambda: wrong # of arguments");
        LASSERT(IS_SXP(at(1)), "lambda: argument list is not s-expr");
        for (auto v : AS_VEC(at(1))) {
            LASSERT(IS_SYM(v), "lambda: non-symbol in argument list");
        }
        return lval(
            LVAL_FUN,
            lval_fun {at(1), at(2), &e}
        );
    }

    // not a special form, proceed normally
    lval f = at(0).eval(e);
    LASSERT(
        IS_BLT(f) || IS_FUN(f),
        "s-expr: invalid procedure"
    );

    vector<lval> res;
    for (int i = 1; i < size(); i++) {
        res.push_back(at(i).eval(e));
    }
    lval tail = SXP_VAL(res);

    if (IS_BLT(f)) {
        return AS_BLT(f)(tail, e);
    } else {
        lval_fun fun = AS_FUN(f);
        // actually, this lenv needs to be allocated on the heap
        lenv env;
        env.outer = fun.env;
        for (int i = 0; i < fun.params.size(); i++) {
            env.emplace(AS_SYM(fun.params[i]), tail[i]);
        }
        return fun.body.eval(env);
    }
}

lval lval::eval(lenv& e) const {
    if (type == LVAL_SYM) {
        lenv* env = &e;
        do {
            if (env->count(*as.sym) > 0) return env->at(*as.sym);
            env = env->outer;
        } while (env);
        throw runtime_error(
            "eval: undefined variable '" + *as.sym + "'");
    }
    if (type == LVAL_SXP) return eval_sexpr(e);
    return *this;
}

bool lval::is_sym(const string& symbol) const {
    return type == LVAL_SYM && *as.sym == symbol;
}

lval lval::tail() const {
    return SXP_VAL(
        std::vector<lval> (
            as.vec->begin() + 1, 
            as.vec->end()
        )
    );
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
    e.emplace("+", BLT_VAL(builtin_add));
    e.emplace("-", BLT_VAL(builtin_sub));
    e.emplace("*", BLT_VAL(builtin_mul));
    e.emplace("/", BLT_VAL(builtin_div));
    e.emplace("list", BLT_VAL(builtin_list));
    e.emplace("head", BLT_VAL(builtin_head));
    e.emplace("tail", BLT_VAL(builtin_tail));
    e.emplace("eval", BLT_VAL(builtin_eval));
    e.emplace("join", BLT_VAL(builtin_join));
}

ostream& lval::print(ostream& out) const {
    switch (type) {
    case LVAL_NUM: out << as.num; break;
    case LVAL_SYM: out << *as.sym; break;
    case LVAL_BLT: out << "<builtin>"; break;
    case LVAL_FUN: out << "<lambda>"; break;
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

lval lenv::at(const std::string& str) {
    return env.at(str);
}

int lenv::count(const std::string& str) {
    return env.count(str);
}

void lenv::emplace(const std::string& str, lval v) {
    env.emplace(str, v);
}