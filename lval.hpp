#ifndef lval_h
#define lval_h

#include <iostream>
#include <vector>
#include <string>
using namespace std;

enum LvalType {
    LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR
};

struct lval {
    LvalType type;
    union {
        double num;
        string* err;
        string* sym;
        vector<lval>* cell;
    } as;
    lval(LvalType type, double x);
    lval(LvalType type, const string& m);
    lval(LvalType type);
    ~lval();
    ostream& print(ostream& out) const;
    ostream& println(ostream& out) const;
    friend ostream& operator<<(ostream& out, const lval& v);
};

#define NUM_VAL(x) lval(LVAL_NUM, x)
#define SYM_VAL(x) lval(LVAL_SYM, x)
#define ERR_VAL(x) lval(LVAL_ERR, x)
#define SXP_VAL()  lval(LVAL_SEXPR)

#endif