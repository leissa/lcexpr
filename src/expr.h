#pragma once

#include <cassert>
#include <cstdint>

#include <ostream>
#include <span>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct World;

enum class Tag {
    Lit, Id,      // 0-ary
    Minus,        // unary
    Add, Sub, Eq, // binary
    Select,       // ternary
};

std::string tag2str(Tag);

template<class T>
struct GIDHash {
    size_t operator()(T p) const { return p->gid; }; // TODO bad hash function
};

template<class T>
struct GIDEq {
    bool operator()(T a, T b) const { return a->gid == b->gid; }
};

template<class T>
struct GIDLt {
    bool operator()(T a, T b) const { return a->gid < b->gid; }
};

struct Expr {
    Expr(World&, Tag tag, std::span<const Expr*> ops, uint64_t stuff = 0);

    static bool equal(const Expr*, const Expr*);
    std::ostream& dump(std::ostream&) const;
    std::ostream& dump() const;
    std::ostream& dot(std::ostream&) const;
    std::ostream& dot() const;
    std::string name() const {
        if (tag == Tag::Lit) return std::to_string(stuff);
        if (tag == Tag::Id) return std::string(1, (char)stuff);
        return tag2str(tag);
    }
    std::string str() const { return std::string("\"") + name() + std::string("\""); }

    World& world;
    size_t gid;
    Tag tag;
    std::vector<const Expr*> ops;
    uint64_t stuff;
    size_t hash;

    /// @name Splay Tree
    ///@{
    const Expr* parent() const { return lc.p && (lc.p->lc.l() == this || lc.p->lc.r() == this) ? lc.p : nullptr; }
    const Expr* path_parent() const { return lc.p && (lc.p->lc.l() != this && lc.p->lc.r() != this) ? lc.p : nullptr; }
    template<size_t l>
    void rot() const;
    void rol() const { return rot<0>(); }
    void ror() const { return rot<1>(); }
    void splay() const;
    ///@}

    /// @name Link/Cut Tree
    ///@{
    void expose() const;            ///< Make `this` the root and the leftmost node in its splay tree.
    void link(const Expr* p) const; ///< Make `this` a child of @p p%arent.
    void cut() const;               ///< Cut `this` from its parent.
    ///@}

    struct LC {
        const Expr* l() const { return child[0]; } // deeper/down
        const Expr* r() const { return child[1]; } // shallower/up

        const Expr* p = nullptr; // parent or path-parent
        std::array<const Expr*, 2> child = {nullptr, nullptr};
    } mutable lc; // intrusive Link/Cut Tree
};

using ExprSet = std::unordered_set<const Expr*, GIDHash<const Expr*>, GIDEq<const Expr*>>;
template<class T>
using ExprMap = std::unordered_map<const Expr*, T, GIDHash<const Expr*>, GIDEq<const Expr*>>;
