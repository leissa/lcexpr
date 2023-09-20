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
    Lit, Id,            // 0-ary
    Minus,              // unary
    Add, Sub, Mul, Eq,  // binary
    Select,             // ternary
    BB,                 // mut unary
    Jmp, Br,            // CF: binary + ternary
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
    Expr(World&);

    static bool equal(const Expr*, const Expr*);
    std::ostream& dump(std::ostream&) const;
    std::ostream& dump() const;

    /// @name GraphViz' Dot output
    ///@{
    void dot() const;
    void dot(std::string) const;
    std::ostream& dot(std::ostream&) const;
    ///@}

    std::string name() const {
        if (tag == Tag::Lit) return std::to_string(stuff);
        if (tag == Tag::Id) return std::string(1, (char)stuff);
        return tag2str(tag);
    }
    std::string str() const { return std::string("\"") + std::to_string(gid) + ": " + name() + std::string("\""); }
    std::string str2() const { return std::string("\"_") + std::to_string(gid) + ": " + name() + std::string("\""); }

    World& world;
    size_t gid;
    bool mut;
    Tag tag;
    std::vector<const Expr*> ops;
    uint64_t stuff;
    size_t hash;

    /// @name Splay Tree
    ///@{
    const Expr* parent() const { return lc.parent && (lc.parent->lc.left == this || lc.parent->lc.right == this) ? lc.parent : nullptr; }
    const Expr* path_parent() const { return lc.parent && (lc.parent->lc.left != this && lc.parent->lc.right != this) ? lc.parent : nullptr; }
    const Expr* root() const;
    static const Expr* lca(const Expr* a, const Expr* b);
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

    struct {
        const Expr*& child(size_t i) { return i == 0 ? left : right; }
        const Expr* parent = nullptr; ///< parent or path-parent
        const Expr* left   = nullptr; ///< deeper/down
        const Expr* right  = nullptr; ///< shallower/up
    } mutable lc; // intrusive Link/Cut Tree
};

using ExprSet = std::unordered_set<const Expr*, GIDHash<const Expr*>, GIDEq<const Expr*>>;
template<class T>
using ExprMap = std::unordered_map<const Expr*, T, GIDHash<const Expr*>, GIDEq<const Expr*>>;
