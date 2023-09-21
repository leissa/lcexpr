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
        auto ab = w.add(a, b);
        auto sel = w.select(eq, ab, w.add(w.lit(2), w.lit(3)));
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

        assert(w.lit(0)->lca(w.lit(1)) == eq);
        assert(w.lit(0)->lca(eq) == eq);
        assert(w.lit(0)->lca(a) == sel);
        assert(w.lit(0)->lca(w.lit(23)) == nullptr);

        w.lit(1)->cut();
        auto z = w.id('z');
        const_cast<Expr*>(ab)->ops[1] = z;
        z->link(ab);
        sel->dot();
    }
#if 0
    {
        World w;
        w.lit(0);
        auto _1 = w.id('1');
        auto _2 = w.id('2');
        auto _3 = w.id('3');
        auto _4 = w.id('4');
        auto _5 = w.id('5');
        auto _6 = w.id('6');
        auto _7 = w.id('7');
        auto _8 = w.id('8');
        auto _9 = w.id('9');
        auto _0 = w.id('0');

        auto link = [&](const Expr* c, const Expr* p) {
            p->left_ = c;
            c->parent_ = p;
        };

        link(_9, _0);
        link(_8, _9);
        link(_7, _8);
        link(_6, _7);
        link(_5, _6);
        link(_4, _5);
        link(_3, _4);
        link(_2, _3);
        link(_1, _2);

        _0->dot();
        _1->splay();
        _1->dot();
        _2->splay();
        _2->dot();
    }
#endif
}
