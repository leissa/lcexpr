#include "expr.h"

#include <iostream>

#include "world.h"

std::string tag2str(Tag tag) {
    switch (tag) {
        case Tag::Zero:   return "0";
        case Tag::One:    return "1";
        case Tag::Minus:  return "-";
        case Tag::Add:    return "+";
        case Tag::Sub:    return "-";
        case Tag::Eq:     return "==";
        case Tag::Select: return "?:";
        default:          return "<unknonw>";
    }
}

Expr::Expr(World& world, Tag tag, std::span<const Expr*> ops)
    : world(world)
    , gid(world.next_gid())
    , tag(tag)
    , ops(ops.begin(), ops.end())
    , hash(size_t(tag)) {
    for (auto op : ops) hash ^= op->gid << 1;
}

bool Expr::equal(const Expr* e1, const Expr* e2) {
    bool res = e1->tag == e2->tag && e1->ops.size() == e2->ops.size();
    size_t n = e1->ops.size();
    for (size_t i = 0; i != n && res; ++i) res &= e1->ops[i]->gid == e2->ops[i]->gid;
    return res;
}

std::ostream& Expr::dump(std::ostream& o) const {
    o << '(' << tag2str(tag);
    for (auto op : ops) op->dump(o << " ");
    return o << ')';
}

std::ostream& Expr::dump() const { return dump(std::cout) << std::endl; }

/*
 * Splay Tree
 */

void rot_down(const Expr* x) {
    auto y = x->lc.up;

    if (y) {
        x->lc.up = y->lc.down;
        if (y->lc.down) y->lc.down->lc.parent = x;
        y->lc.parent = x->lc.parent;
    }

    if (!x->lc.parent)
        ;// root = y;
    else if (x == x->lc.parent->lc.down)
        x->lc.parent->lc.down = y;
    else
        x->lc.parent->lc.up = y;

    if (y) y->lc.down = x;
    x->lc.parent = y;
}

void rot_up(const Expr* x) {
    auto y = x->lc.down;

    if (y) {
        x->lc.down = y->lc.up;
        if (y->lc.up) y->lc.up->lc.parent = x;
        y->lc.parent = x->lc.parent;
    }

    if (!x->lc.parent)
        ;// root = y;
    else if (x == x->lc.parent->lc.down)
        x->lc.parent->lc.down = y;
    else
        x->lc.parent->lc.up = y;

    if (y) y->lc.up = x;
    x->lc.parent = y;
}
