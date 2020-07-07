#ifndef lval_h
#define lval_h

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

struct lval;
using lenv = std::unordered_map<std::string, lval>;
using lbuiltin = lval (*)(lval, lenv&);
using lfun = lval (*)(lval, lval, lval, lenv&);

enum LvalType {
    LVAL_NUM, LVAL_SYM, LVAL_BLT, LVAL_FUN, LVAL_SXP
};

lval builtin_add(lval v, lenv& e);
lval builtin_sub(lval v, lenv& e);
lval builtin_mul(lval v, lenv& e);
lval builtin_div(lval v, lenv& e);
lval builtin_head(lval v, lenv& e);
lval builtin_tail(lval v, lenv& e);
lval builtin_list(lval v, lenv& e);
lval builtin_eval(lval v, lenv& e);
lval builtin_join(lval v, lenv& e);

void lenv_add_builtins(lenv& e);

struct lval {
    const LvalType type;
    union {
        const double num;
        const std::string* const sym;
        const lbuiltin fun;
        const std::vector<lval>* const vec;
    } as;
    lval(LvalType type, double x);
    lval(LvalType type, const std::string& m);
    lval(LvalType type, lbuiltin fun);
    lval(LvalType type, const std::vector<lval>& v);

    lval operator[](int i) const { return (*as.vec)[i]; }
    lval at(int i) const { return (*as.vec)[i]; }
    int size() const { return as.vec->size(); }
    lval tail() const {
        return lval(LVAL_SXP, std::vector<lval> (as.vec->begin() + 1, as.vec->end()));
    }

    lval eval_sexpr(lenv& e) const;
    lval eval(lenv& e) const;
    lval builtin_op(const std::string& op, lenv& e);

    std::ostream& print(std::ostream& out) const;
    std::ostream& println(std::ostream& out) const;
    friend std::ostream& operator<<(std::ostream& out, const lval& v);
private:
    bool is_quote() const;
    bool is_def() const;
};

#define NUM_VAL(x) lval(LVAL_NUM, (x))
#define SYM_VAL(x) lval(LVAL_SYM, (x))
#define FUN_VAL(x) lval(LVAL_BLT, (x))
#define SXP_VAL(x) lval(LVAL_SXP, (x))

#define AS_NUM(x)  ( (x).as.num)
#define AS_SYM(x)  (*(x).as.sym)
#define AS_FUN(x)  ( (x).as.fun)
#define AS_VEC(x)  (*(x).as.vec)

#define IS_NUM(x)  ((x).type == LVAL_NUM)
#define IS_SYM(x)  ((x).type == LVAL_SYM)
#define IS_FUN(x)  ((x).type == LVAL_BLT)
#define IS_SXP(x)  ((x).type == LVAL_SXP)

#endif