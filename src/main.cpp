#include <iostream>

#include "world.h"

void link(const Expr* c, const Expr* p) {
    c->lc.p = p;
    p->lc.l = c;
}

void rink(const Expr* c, const Expr* p) {
    c->lc.p = p;
    p->lc.r = c;
}

int main() {
    World w;

    auto a = w.id('a');
    auto b = w.id('b');
    auto eq = w.eq(w.lit(0), w.lit(1));
    auto sel = w.select(eq, w.add(a, b), w.add(w.lit(2), w.lit(3)));
    sel->dot();
    w.lit(1)->expose();
    sel->dot();
    eq->expose();
    sel->dot();
    w.lit(1)->expose();
    sel->dot();
    w.lit(5)->expose();
    sel->dot();
    a->expose();
    sel->dot();
    b->expose();
    sel->dot();
}
