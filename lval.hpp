#ifndef lval_h
#define lval_h

#include <iostream>
#include <vector>
#include <string>

enum LvalType {
    LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR
};

struct lval {
    LvalType type;
    union {
        double num;
        std::string* err;
        std::string* sym;
        std::vector<lval>* cell;
    } as;
    lval(LvalType type, double x);
    lval(LvalType type, const std::string& m);
    lval(LvalType type);
    ~lval();

    lval pop(int i);
    lval take(int i);

    std::ostream& print(std::ostream& out) const;
    std::ostream& println(std::ostream& out) const;
    friend std::ostream& operator<<(std::ostream& out, const lval& v);
};

#define NUM_VAL(x) lval(LVAL_NUM, x)
#define SYM_VAL(x) lval(LVAL_SYM, x)
#define ERR_VAL(x) lval(LVAL_ERR, x)
#define SXP_VAL()  lval(LVAL_SEXPR)

#endif