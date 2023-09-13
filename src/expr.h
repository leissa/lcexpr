#pragma once

#include <cassert>
#include <cstdint>

#include <ostream>
#include <span>
#include <string>
#include <vector>

struct World;

enum class Tag {
    Lit, Id,      // 0-ary
    Minus,        // unary
    Add, Sub, Eq, // binary
    Select,       // ternary
};

std::string tag2str(Tag);

struct Expr {
    Expr(World&, Tag tag, std::span<const Expr*> ops, uint64_t stuff = 0);

    static bool equal(const Expr*, const Expr*);
    std::ostream& dump(std::ostream&) const;
    std::ostream& dump() const;

    World& world;
    size_t gid;
    Tag tag;
    std::vector<const Expr*> ops;
    uint64_t stuff;
    size_t hash;

    /**
     *  Left                        Right
     *      p              p            p            p
     *      |              |            |            |
     *      x              c            x            c
     *     / \     ->     / \          / \    ->    / \
     *    a   c          x   d        c   a        d   x
     *       / \        / \          / \              / \
     *      b   d      a   b        d   b            b   a
     *
     */
    template<size_t l>
    static void rot(const Expr* x) {
        constexpr size_t r = (l + 1) % 2;
        auto p = x->lc.p;
        auto c = x->lc.child[r];
        x->lc.p = c;

        if (c) {
            auto b = c->lc.child[l];
            x->lc.child[r] = b;
            if (b) b->lc.p = x;
            c->lc.p = p;
            c->lc.child[l] = x;
        }

        if (!p) {
            // x is new root
        } else if (p->lc.child[l] == x) {
            p->lc.child[l] = c;
        } else {
            p->lc.child[r] = c;
        }
    }

    void rol(const Expr* x) { return rot<0>(x); }
    void ror(const Expr* x) { return rot<1>(x); }

    void splay(const Expr* x) {
        while (auto p = x->lc.p) {
            if (auto pp = p->lc.p) {
                if (p->lc.child[LC::L] == x && pp->lc.child[LC::L] == p) {
                    ror(pp);
                    ror(p);
                } else if (p->lc.child[LC::R] == x && pp->lc.child[LC::R] == p) {
                    rol(pp);
                    rol(p);
                } else if (p->lc.child[LC::L] == x && pp->lc.child[LC::R] == p) {
                    ror(p);
                    rol(p);
                } else {
                    rol(p);
                    ror(p);
                }
            } else if (p->lc.child[LC::L] == x) {
                ror(p);
            } else {
                rol(p);
            }
        }
    }

    struct LC {
        enum { L, R };
        const Expr* p = nullptr;
        std::array<const Expr*, 2> child = {nullptr, nullptr};
    } mutable lc; // intrusive Link/Cut Tree
};
