#include "lval.hpp"
#include <cstring>
#include <stdexcept>
using namespace std;

lval::lval(LvalType type, double x) : type(type), as {.num = x} {}

lval::lval(LvalType type, bool truth): type(type), as {.truth = truth} {}

lval::lval(LvalType type, const string& m) : type(type), as {.sym = new string(m)} {}

lval::lval(LvalType type, lbuiltin blt): type(type), as {.blt = blt} {}

lval::lval(LvalType type, const lval_fun& fun) : type(type), as {.fun = new lval_fun(fun)} {}

lval::lval(LvalType type, const vector<lval>& v) : type(type), as {.vec = new vector<lval> (v)} {}

#define LASSERT(cond, err) \
    if (!(cond)) { throw runtime_error(err); }

#define LASSERT_NUM(n, err) \
    LASSERT(size() == (n), (err) + string(": wrong # of arguments"))

#define LASSERT_TYPE(op, type, typestr) \
    for (auto v: AS_VEC(at(1))) { \
        LASSERT(IS_##type(v), string(op) + ": " typestr " argument required"); \
    }

lval lval::eval_sexpr(lenv& e) const {
    LASSERT(nonempty(), "s-expr: expression required");
    
    // quote
    if (head().is_sym("quote")) {
        LASSERT_NUM(2, "quote");
        lval quoted = at(1);
        return quoted;
    }
    
    // define
    if (head().is_sym("define")) {
        LASSERT_NUM(3, "define");
        if (IS_SYM(at(1))) {
            lval name = at(1);
            lval res = at(2).eval(e);
            e.emplace(AS_SYM(name), res);
            return res;
        } else if (IS_SXP(at(1))) {
            LASSERT(at(1).nonempty(), "define: function name required");
            LASSERT_TYPE("define", SYM, "symbolic");
            lval name = at(1).at(0);
            lval params = at(1).tail();
            lval body = at(2);
            lval res = lval(
                LVAL_FUN,
                lval_fun {&e, params, body}
            );
            e.emplace(AS_SYM(name), res);
            return res;
        } else {
            throw runtime_error("define: invalid definition target");
        }
    }
    
    // lambda
    if (head().is_sym("lambda")) {
        LASSERT_NUM(3, "lambda");
        LASSERT(IS_SXP(at(1)), "lambda: argument list must be s-expr");
        LASSERT_TYPE("lambda", SYM, "symbolic");
        return lval(
            LVAL_FUN,
            lval_fun {&e, at(1), at(2)}
        );
    }

    // if
    if (head().is_sym("if")) {
        LASSERT(size() == 3 || size() == 4, "if: wrong # of args");
        lval cond = at(1).eval(e);
        LASSERT(IS_BOOL(cond), "if: condition must be boolean");
        if (AS_BOOL(cond)) {
            return at(2).eval(e);
        } else if (size() == 4) {
            return at(3).eval(e);
        } else {
            return SXP_VAL(vector<lval> ());
        }
    }

    // not a special form, proceed normally
    lval f = head().eval(e);
    LASSERT(IS_BLT(f) || IS_FUN(f), "s-expr: invalid procedure");

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
        env.outer = fun.outer;
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
            else env = env->outer;
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

#define FASSERT_TYPE(op, type, typestr) \
    for (auto w: AS_VEC(v)) { \
        LASSERT(IS_##type(w), string(op) + ": " typestr " argument required"); \
    }

#define FASSERT_TYPE_NUM(op) FASSERT_TYPE(op, NUM, "numeric")

#define FASSERT_NUM(n, err) \
    LASSERT(v.size() == (n), (err + string(": wrong # of args")))

lval builtin_add(lval v, lenv& e) {
    FASSERT_TYPE_NUM("+");
    double x = 0;
    for (auto w: AS_VEC(v)) x += AS_NUM(w);
    return NUM_VAL(x);
}

lval builtin_sub(lval v, lenv& e) {
    FASSERT_TYPE_NUM("-");
    if (v.size() == 1) return NUM_VAL(-AS_NUM(v.at(0)));
    else if (v.size() == 2) {
        return NUM_VAL(AS_NUM(v.at(0)) - AS_NUM(v.at(1)));
    } else {
        throw runtime_error("-: too many arguments");
    }
}

lval builtin_mul(lval v, lenv& e) {
    FASSERT_TYPE_NUM("*");
    double x = 1;
    for (auto w: AS_VEC(v)) x *= AS_NUM(w);
    return NUM_VAL(x);
}

lval builtin_div(lval v, lenv& e) {
    FASSERT_TYPE_NUM("/");
    FASSERT_NUM(2, "/");
    LASSERT(AS_NUM(v.at(1)) != 0, "/: division by 0");
    return NUM_VAL(AS_NUM(v.at(0)) / AS_NUM(v.at(1)));
}

lval builtin_lt(lval v, lenv& e) {
    FASSERT_NUM(2, "<");
    FASSERT_TYPE_NUM("<");
    return BOOL_VAL(AS_NUM(v.at(0)) < AS_NUM(v.at(1)));
}

lval builtin_gt(lval v, lenv& e) {
    FASSERT_NUM(2, ">");
    FASSERT_TYPE_NUM(">");
    return BOOL_VAL(AS_NUM(v.at(0)) > AS_NUM(v.at(1)));
}

lval builtin_le(lval v, lenv& e) {
    FASSERT_NUM(2, "<=");
    FASSERT_TYPE_NUM("<=");
    return BOOL_VAL(AS_NUM(v.at(0)) <= AS_NUM(v.at(1)));
}

lval builtin_ge(lval v, lenv& e) {
    FASSERT_NUM(2, ">=");
    FASSERT_TYPE_NUM(">=");
    return BOOL_VAL(AS_NUM(v.at(0)) >= AS_NUM(v.at(1)));
}

lval builtin_eq(lval v, lenv& e) {
    FASSERT_NUM(2, "=");
    FASSERT_TYPE_NUM("=");
    return BOOL_VAL(AS_NUM(v.at(0)) == AS_NUM(v.at(1)));
}

lval builtin_ne(lval v, lenv& e) {
    FASSERT_NUM(2, "<>");
    FASSERT_TYPE_NUM("<>");
    return BOOL_VAL(AS_NUM(v.at(0)) != AS_NUM(v.at(1)));
}

lval builtin_head(lval v, lenv& e) {
    FASSERT_NUM(1, "head");
    LASSERT(IS_SXP(v.at(0)), "head: list required");
    LASSERT(v.at(0).nonempty(), "head: empty list");
    return v.at(0).at(0);
}

lval builtin_tail(lval v, lenv& e) {
    FASSERT_NUM(1, "tail");
    LASSERT(IS_SXP(v.at(0)), "tail: list required");
    LASSERT(v.at(0).nonempty(), "tail: empty list");
    return v.at(0).tail();
}

lval builtin_join(lval v, lenv& e) {
    vector<lval> x;
    for (auto& u : AS_VEC(v)) {
        LASSERT(
            u.type == LVAL_SXP,
            "join: list required"
        );
        for (auto& w : AS_VEC(u)) { x.emplace_back(w); }
    }
    return SXP_VAL(x);
}

lval builtin_list(lval v, lenv& e) {
    return v;
}

lval builtin_eval(lval v, lenv& e) {
    FASSERT_NUM(1, "eval");
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
    e.emplace("<",  BLT_VAL(builtin_lt));
    e.emplace(">",  BLT_VAL(builtin_gt));
    e.emplace("<=", BLT_VAL(builtin_le));
    e.emplace(">=", BLT_VAL(builtin_ge));
    e.emplace("=",  BLT_VAL(builtin_eq));
    e.emplace("<>", BLT_VAL(builtin_ne));
}

ostream& operator<<(ostream& out, const lval& v) {
    switch (v.type) {
    case LVAL_NUM: out << AS_NUM(v); break;
    case LVAL_BOOL: out << (AS_BOOL(v) ? "#t" : "#f"); break;
    case LVAL_SYM: out << AS_SYM(v); break;
    case LVAL_BLT: out << "<builtin function>"; break;
    case LVAL_FUN:
        out << "(lambda " << AS_FUN(v).params << " ...)";
        break;
    case LVAL_SXP:
        out << "(";
        for (int i = 0; i < v.size(); i++) {
            out << v[i];
            if (i < v.size() - 1) out << " ";
        }
        out << ")";
    }
    return out;
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