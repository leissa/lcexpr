#include <iostream>

#include "world.h"
#include "link_cut_tree.h"

struct Test : public LinkCutTree<Test> {
    int i = 0;
};

template<bool flip>
void test_splay() {
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

    _9->splay_link<flip>(_0);
    _8->splay_link<flip>(_9);
    _7->splay_link<flip>(_8);
    _6->splay_link<flip>(_7);
    _5->splay_link<flip>(_6);
    _4->splay_link<flip>(_5);
    _3->splay_link<flip>(_4);
    _2->splay_link<flip>(_3);
    _1->splay_link<flip>(_2);

    _0->dot();
    _1->splay();
    _1->dot();
    _2->splay();
    _2->dot();
}

int main() {
    test_splay<false>();
    test_splay<true>();

    Test a, b;
    b.link(&a);
    auto r = a.root();
    r->i++;
    if (auto p = a.splay_parent()) p->i++;

    {
        World w;
        auto x = w.id('x');
        auto p = w.add(x, x);
        p->dot();
        p->expose();
        p->dot();
        x->expose();
        p->dot();
    }

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
        ab->link(z);
        sel->dot();
    }
    {
        World w;
        auto x = w.id('x');
        auto y = w.id('y');
        auto p = w.bb();
        auto m = w.bb();
        auto a = w.add(p, m);
        p->set(x);
        m->set(y);
        a->dot();
        x->expose();
        a->dot();
    }
    {   // if diamond
        World w;
        auto x = w.id('x');
        auto p = w.bb();
        auto m = w.bb();
        auto a = w.add(p, m);
        p->set(x);
        m->set(x);
        a->dot();
        x->expose();
        a->dot();

    }
    {   // if diamond
        World w;
        auto start = w.bb();
        auto cons  = w.bb();
        auto alt   = w.bb();
        auto next  = w.bb();
        auto c = w.id('c'); // cond
        auto p = w.id('p'); // phi
        auto r = w.id('r'); // ret
        start->set(w.br(c, cons, alt));
        cons ->set(w.jmp(next, w.lit(23)));
        alt  ->set(w.jmp(next, w.lit(42)));
        next ->set(w.jmp(r, p));
        start->dot();
    }
    {   // loop
        World w;
        auto start = w.bb();
        auto head  = w.bb();
        auto body  = w.bb();
        auto exit  = w.bb();
        auto a = w.id('a'); // begin
        auto i = w.id('i'); // phi/i
        auto r = w.id('r'); // ret
        start->set(w.jmp(a, head));
        head ->set(w.br(w.eq(i, w.lit(42)), exit, body));
        body ->set(w.jmp(head, w.add(i, w.lit(1))));
        exit ->set(w.jmp(r, i));
        start->dot();
        body->expose();
        start->dot();
    }
    {
        World w;
        auto a = w.bb();
        auto b = w.bb();
        a->set(b);
        a->dot();
        b->set(a);
        a->dot();
    }
}
