#ifndef lval_h
#define lval_h

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

struct lval;
struct lenv;
struct lval_fun;
using lbuiltin = lval (*)(lval, lenv&);

enum LvalType {
    LVAL_NUM, LVAL_BOOL, LVAL_SYM, LVAL_BLT, LVAL_FUN, LVAL_SXP
};

struct lval {
    const LvalType type;
    union {
        const double num;
        const bool truth;
        const std::string* const sym;
        const lbuiltin blt;
        const lval_fun* const fun;
        const std::vector<lval>* const vec;
    } as;
    lval(LvalType type, double x);
    lval(LvalType type, bool truth);
    lval(LvalType type, const std::string& m);
    lval(LvalType type, lbuiltin blt);
    lval(LvalType type, const lval_fun& fun);
    lval(LvalType type, const std::vector<lval>& v);

    lval operator[](int i) const { return (*as.vec)[i]; }
    lval at(int i) const { return (*as.vec)[i]; }
    int size() const { return as.vec->size(); }
    lval tail() const;

    lval eval_sexpr(lenv& e) const;
    lval eval(lenv& e) const;
    lval builtin_op(const std::string& op, lenv& e);

    std::ostream& print(std::ostream& out) const;
    std::ostream& println(std::ostream& out) const;
    friend std::ostream& operator<<(std::ostream& out, const lval& v);
private:
    bool is_sym(const std::string&) const;
};

#define NUM_VAL(x) lval(LVAL_NUM, (x))
#define BOOL_VAL(x) lval(LVAL_BOOL, (x))
#define SYM_VAL(x) lval(LVAL_SYM, (x))
#define BLT_VAL(x) lval(LVAL_BLT, (x))
#define FUN_VAL(x) lval(LVAL_FUN, (x))
#define SXP_VAL(x) lval(LVAL_SXP, (x))

#define AS_NUM(x)  ( (x).as.num)
#define AS_BOOL(x) ( (x).as.truth)
#define AS_SYM(x)  (*(x).as.sym)
#define AS_BLT(x)  ( (x).as.blt)
#define AS_FUN(x)  (*(x).as.fun)
#define AS_VEC(x)  (*(x).as.vec)

#define IS_NUM(x)  ((x).type == LVAL_NUM)
#define IS_BOOL(x) ((x).type == LVAL_BOOL)
#define IS_SYM(x)  ((x).type == LVAL_SYM)
#define IS_BLT(x)  ((x).type == LVAL_BLT)
#define IS_FUN(x)  ((x).type == LVAL_FUN)
#define IS_SXP(x)  ((x).type == LVAL_SXP)

lval builtin_add(lval v, lenv& e);
lval builtin_sub(lval v, lenv& e);
lval builtin_mul(lval v, lenv& e);
lval builtin_div(lval v, lenv& e);
lval builtin_head(lval v, lenv& e);
lval builtin_tail(lval v, lenv& e);
lval builtin_list(lval v, lenv& e);
lval builtin_eval(lval v, lenv& e);
lval builtin_join(lval v, lenv& e);
lval builtin_lt(lval v, lenv& e);
lval builtin_gt(lval v, lenv& e);
lval builtin_le(lval v, lenv& e);
lval builtin_ge(lval v, lenv& e);
lval builtin_eq(lval v, lenv& e);
lval builtin_ne(lval v, lenv& e);
void lenv_add_builtins(lenv& e);

struct lenv {
    lenv* outer = nullptr;
    std::unordered_map<std::string, lval> env;
    lval at(const std::string& str);
    int count(const std::string& str);
    void emplace(const std::string& str, lval v);
};

struct lval_fun {
    lval params;
    lval body;
    lenv* env;
};

#endif