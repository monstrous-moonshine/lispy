#ifndef lval_h
#define lval_h

#include <iostream>
#include <vector>
#include <string>

enum LvalType {
    LVAL_NUM, LVAL_SYM, LVAL_SXP
};

struct lval {
    const LvalType type;
    union {
        const double num;
        const std::string* const sym;
        const std::vector<lval>* const vec;
    } as;
    lval(LvalType type, double x);
    lval(LvalType type, const std::string& m);
    lval(LvalType type, const std::vector<lval>& v);

    lval operator[](int i) const { return (*as.vec)[i]; }
    lval at(int i) const { return (*as.vec)[i]; }
    int size() const { return as.vec->size(); }
    lval tail() const {
        return lval(LVAL_SXP, std::vector<lval> (as.vec->begin() + 1, as.vec->end()));
    }

    lval eval_sexpr() const;
    lval eval() const;
    lval builtin(const std::string& func);
    lval builtin_op(const std::string& op);
    lval builtin_head();
    lval builtin_tail();
    lval builtin_list();
    lval builtin_eval();
    lval builtin_join();

    std::ostream& print(std::ostream& out) const;
    std::ostream& println(std::ostream& out) const;
    friend std::ostream& operator<<(std::ostream& out, const lval& v);
private:
    bool is_quote() const;
};

struct lenv;
using lbuiltin = lval (*)();

#define NUM_VAL(x) lval(LVAL_NUM, (x))
#define SYM_VAL(x) lval(LVAL_SYM, (x))
#define SXP_VAL(x) lval(LVAL_SXP, (x))

#define AS_NUM(x)  ( (x).as.num)
#define AS_SYM(x)  (*(x).as.sym)
#define AS_VEC(x)  (*(x).as.vec)

#define IS_NUM(x)  ((x).type == LVAL_NUM)
#define IS_SYM(x)  ((x).type == LVAL_SYM)
#define IS_SXP(x)  ((x).type == LVAL_SXP)

#endif