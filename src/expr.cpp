#include "expr.h"

#include <iostream>

#include "world.h"

std::string tag2str(Tag tag) {
    switch (tag) {
        case Tag::Id:     return "<id>";
        case Tag::Lit:    return "<lit>";
        case Tag::Minus:  return "-";
        case Tag::Add:    return "+";
        case Tag::Sub:    return "-";
        case Tag::Eq:     return "==";
        case Tag::Select: return "?:";
        default:          return "<unknonw>";
    }
}

Expr::Expr(World& world, Tag tag, std::span<const Expr*> ops, uint64_t stuff)
    : world(world)
    , gid(world.next_gid())
    , tag(tag)
    , ops(ops.begin(), ops.end())
    , stuff(stuff)
    , hash(size_t(tag)) {
    hash ^= stuff << 1;
    for (auto op : ops) hash ^= op->gid << 1;
}

bool Expr::equal(const Expr* e1, const Expr* e2) {
    bool res = e1->tag == e2->tag && e1->stuff == e2->stuff && e1->ops.size() == e2->ops.size();
    size_t n = e1->ops.size();
    for (size_t i = 0; i != n && res; ++i) res &= e1->ops[i]->gid == e2->ops[i]->gid;
    return res;
}

std::ostream& Expr::dump(std::ostream& o) const {
    if (tag == Tag::Lit) return o << stuff;
    if (tag == Tag::Id) return o << (char)stuff;
    o << "(" << tag2str(tag);
    for (auto op : ops) op->dump(o << " ");
    return o << ")";
}

std::ostream& Expr::dump() const { return dump(std::cout) << std::endl; }

/*
 * Splay Tree
 */

/**
 *  Left                        Right
 *      p              p            p            p
 *      |              |            |            |
 *     this            c           this          c
 *     / \     ->     / \          / \    ->    / \
 *    a   c         this d        c   a        d  this
 *       / \        / \          / \              / \
 *      b   d      a   b        d   b            b   a
 *
 */
template<size_t l>
void Expr::rot() const {
    constexpr size_t r = (l + 1) % 2;
    auto p = lc.p;
    auto c = lc.child[r];
    lc.p = c;

    if (c) {
        auto b = c->lc.child[l];
        lc.child[r] = b;
        if (b) b->lc.p = this;
        c->lc.p = p;
        c->lc.child[l] = this;
    }

    if (!p) {
        // this is new root
    } else if (p->lc.child[l] == this) {
        p->lc.child[l] = c;
    } else {
        p->lc.child[r] = c;
    }
}

void Expr::splay() const {
    while (auto p = lc.p) {
        if (auto pp = p->lc.p) {
            if (p->lc.l() == this && pp->lc.l() == p) {
                pp->ror();
                p->ror();
            } else if (p->lc.r() == this && pp->lc.r() == p) {
                pp->rol();
                p->rol();
            } else if (p->lc.l() == this && pp->lc.r() == p) {
                p->ror();
                p->rol();
            } else {
                p->rol();
                p->ror();
            }
        } else if (p->lc.l() == this) {
            p->ror();
        } else {
            p->rol();
        }
    }
}

