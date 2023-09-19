#include "expr.h"

#include <iostream>
#include <fstream>
#include <queue>

#include "world.h"

using namespace std::string_literals;

std::string tag2str(Tag tag) {
    switch (tag) {
        case Tag::Id:     return "<id>";
        case Tag::Lit:    return "<lit>";
        case Tag::Minus:  return "-";
        case Tag::Add:    return "+";
        case Tag::Sub:    return "-";
        case Tag::Mul:    return "*";
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
    for (auto op : ops) {
        hash ^= op->gid << 1;
        op->link(this);
    }
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

void Expr::dot() const {
    static int i = 0;
    dot("out" + std::to_string(i++) + ".dot");
}

void Expr::dot(std::string name) const {
    std::ofstream ofs(name);
    dot(ofs);
}

std::ostream& Expr::dot(std::ostream& o) const {
    ExprSet done;
    std::queue<const Expr*> q;

    auto enqueue = [&q, &done](const Expr* expr) {
        if (done.emplace(expr).second) q.emplace(expr);
    };

    enqueue(this);

    o << "digraph A {" << std::endl;;

    while (!q.empty()) {
        auto expr = q.front();
        q.pop();

        for (auto op : expr->ops) {
            //if (expr->lc.l == op)
                //o << '\t' << expr->str() << " -> " << op->str() << "[color=green];" << std::endl;
            //else if (op->lc.r == expr)
                //o << '\t' << expr->str() << " -> " << op->str() << "[color=red];" << std::endl;
            //else
                o << '\t' << expr->str() << " -> " << op->str() << "[color=black];" << std::endl;
            enqueue(op);
        }
    }

    done.clear();
    enqueue(this);

    while (!q.empty()) {
        auto expr = q.front();
        q.pop();

        if (auto p = expr->parent()) o << '\t' << expr->str2() << " -> " << p->str2() << "[style=dashed];" << std::endl;
        if (auto p = expr->path_parent()) o << '\t' << expr->str2() << " -> " << p->str2() << "[style=dashed,color=gray];" << std::endl;
        if (auto l = expr->lc.l) o << '\t' << expr->str2() << " -> " << l->str2() << "[color=green];" << std::endl;
        if (auto r = expr->lc.r) o << '\t' << expr->str2() << " -> " << r->str2() << "[color=red];" << std::endl;

        for (auto op : expr->ops) enqueue(op);
    }

    return o << "}" << std::endl;;
}

/*
 * Splay Tree
 */

/*
 *  | Left                  | Right                     |
 *  ----------------------------------------------------|
 *  |   p              p    |       p            p      |
 *  |   |              |    |       |            |      |
 *  |  this            c    |      this          c      |
 *  |  / \     ->     / \   |      / \    ->    / \     |
 *  | a   c         this d  |     c   a        d  this  |
 *  |    / \        / \     |    / \              / \   |
 *  |   b   d      a   b    |   d   b            b   a  |
 *  |
 */
template<size_t l>
void Expr::rot() const {
    constexpr size_t r = (l + 1) % 2;
    auto p = lc.p;
    auto ppp = parent();
    auto c = lc.child[r];
    lc.p = c;

    if (c) {
        auto b = c->lc.child[l];
        lc.child[r] = b;
        if (b) b->lc.p = this;
        c->lc.p = p;
        c->lc.child[l] = this;
    }

    if (!ppp) {
        // this is new root
    } else if (p->lc.child[l] == this) {
        p->lc.child[l] = c;
    } else {
        assert(p->lc.child[r] == this);
        p->lc.child[r] = c;
    }
}

void Expr::splay() const {
    while (auto p = parent()) {
        if (auto pp = p->parent()) {
            if (p->lc.l == this && pp->lc.l == p) {         // zig-zig
                pp->ror();
                p->ror();
            } else if (p->lc.r == this && pp->lc.r == p) {  // zag-zag
                pp->rol();
                p->rol();
            } else if (p->lc.l == this && pp->lc.r == p) {  // zig-zag
                p->ror();
                pp->rol();
            } else {                                        // zag-zig
                assert(p->lc.r == this && pp->lc.l == p);
                p->rol();
                pp->ror();
            }
        } else if (p->lc.l == this) {                       // zig
            p->ror();
        } else {                                            // zag
            assert(p->lc.r == this);
            p->rol();
        }
    }
}

/*
 * Link/Cut Tree
 */

void Expr::link(const Expr* up) const {
    expose();
    up->expose();
    up->lc.p = this;
    assert(!lc.r);
    lc.r = up;
}

void Expr::cut() const {
    expose();
    if (auto& r = lc.r) {
        r->lc.p = nullptr;
        r       = nullptr;
    }
}

void Expr::expose() const {
    for (const Expr* expr = this, *prev = nullptr; expr; prev = expr, expr = expr->lc.p) {
        expr->splay();
        assert(!prev || prev->lc.p == expr);
        expr->lc.l = prev;
    }
    splay();
}
