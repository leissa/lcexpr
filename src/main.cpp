#include <iostream>

#include "world.h"
#include "link_cut_tree.h"

struct Test : public LinkCutTree<Test> {
};

//struct ConstTest : LinkCutTree<const Test> {
//};

void foo(const Test* test) {
    test->root();
}

int main() {
    Test a, b;
    a.link(&b);
    a.root();
    a.cut();

    Test x, y;
    x.link(&y);
    foo(&y);
    y.cut();

    {
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

        assert(a->root() == sel);
        assert(b->root() == sel);
        assert(eq->root() == sel);
        assert(w.lit(1)->root() == sel);
        sel->dot();

        Expr::lca(w.lit(0), w.lit(1))->dump();
    }
}
