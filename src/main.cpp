#include <iostream>

#include "world.h"
#include "link_cut_tree.h"

struct Test : public LinkCutTree<Test> {
    int i = 0;
};

int main() {
    Test a, b;
    a.link(&b);
    auto r = a.root();
    r->i++;
    if (auto p = a.splay_parent()) p->i++;

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
        sel->expose();
        sel->dot();

        assert(Expr::lca(w.lit(0), w.lit(1)) == eq);
        assert(Expr::lca(w.lit(0), eq) == eq);
        assert(Expr::lca(w.lit(0), a) == sel);
        assert(Expr::lca(w.lit(0), w.lit(23)) == nullptr);
    }
}
