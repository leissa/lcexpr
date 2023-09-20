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
    , mut(false)
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

Expr::Expr(World& world)
    : world(world)
    , gid(world.next_gid())
    , mut(true)
    , tag(Tag::BB)
    , ops(1, nullptr)
    , stuff(0)
    , hash(gid) {}

bool Expr::equal(const Expr* e1, const Expr* e2) {
    if (e1->mut || e2->mut) return e1 == e2;

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
            //if (expr->lc.left == op)
                //o << '\t' << expr->str() << " -> " << op->str() << "[color=green];" << std::endl;
            //else if (op->lc.rightight == expr)
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
        if (auto l = expr->lc.left) o << '\t' << expr->str2() << " -> " << l->str2() << "[color=green];" << std::endl;
        if (auto r = expr->lc.right) o << '\t' << expr->str2() << " -> " << r->str2() << "[color=red];" << std::endl;

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
    auto p = lc.parent;
    auto ppp = parent();
    auto c = lc.child(r);
    lc.parent = c;

    if (c) {
        auto b = c->lc.child(l);
        lc.child(r) = b;
        if (b) b->lc.parent = this;
        c->lc.parent = p;
        c->lc.child(l) = this;
    }

    if (!ppp) {
        // this is new root
    } else if (p->lc.child(l) == this) {
        p->lc.child(l) = c;
    } else {
        assert(p->lc.child(r) == this);
        p->lc.child(r) = c;
    }
}

void Expr::splay() const {
    while (auto p = parent()) {
        if (auto pp = p->parent()) {
            if (p->lc.left == this && pp->lc.left == p) {           // zig-zig
                pp->ror();
                p->ror();
            } else if (p->lc.right == this && pp->lc.right == p) {  // zag-zag
                pp->rol();
                p->rol();
            } else if (p->lc.left == this && pp->lc.right == p) {   // zig-zag
                p->ror();
                pp->rol();
            } else {                                                // zag-zig
                assert(p->lc.right == this && pp->lc.left == p);
                p->rol();
                pp->ror();
            }
        } else if (p->lc.left == this) {                            // zig
            p->ror();
        } else {                                                    // zag
            assert(p->lc.right == this);
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
    up->lc.parent = this;
    assert(!lc.right);
    lc.right = up;
}

void Expr::cut() const {
    expose();
    if (auto& r = lc.right) {
        r->lc.parent = nullptr;
        r            = nullptr;
    }
}

void Expr::expose() const {
    for (const Expr* expr = this, *prev = nullptr; expr; prev = expr, expr = expr->lc.parent) {
        expr->splay();
        assert(!prev || prev->lc.parent == expr);
        expr->lc.left = prev;
    }
    splay();
}

const Expr* Expr::root() const {
    expose();
    const Expr* expr = this;
    while (auto r = expr->lc.right) expr = r;
    expr->splay();
    return expr;
}

const Expr* Expr::lca(const Expr* a, const Expr* b) {
    if (a == b) return a;
    if (a->root() != b->root()) return nullptr;
    a->expose();
    b->expose();
    return b;
}
