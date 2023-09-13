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
    void rot() const;
    void rol() const { return rot<0>(); }
    void ror() const { return rot<1>(); }
    void splay() const;

    struct LC {
        const Expr* p = nullptr;
        std::array<const Expr*, 2> child = {nullptr, nullptr};
        const Expr* l() const { return child[0]; }
        const Expr* r() const { return child[1]; }
    } mutable lc; // intrusive Link/Cut Tree
};
