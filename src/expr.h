#pragma once

#include <cassert>
#include <cstdint>

#include <ostream>
#include <span>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "link_cut_tree.h"

struct World;

enum class Tag {
    Lit, Id,            // 0-ary
    Minus, Plus,        // unary
    Add, Sub, Mul, Eq,  // binary
    Select,             // ternary
    Jmp, Br,            // CF: binary + ternary
    BB,                 // mut unary
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

struct Expr : public LinkCutTree<const Expr> {
    Expr(World&, Tag tag, std::span<const Expr*> ops, uint64_t stuff = 0);
    Expr(World&);

    void set(const Expr* e) {
        assert(ops.size() == 1);
        assert(ops[0] == nullptr);
        ops[0] = e;
        e->link(this);
    }

    static bool equal(const Expr*, const Expr*);
    std::ostream& dump(std::ostream&) const;
    std::ostream& dump() const;

    /// @name GraphViz' Dot output
    ///@{
    void dot() const;
    void dot(std::string) const;
    std::ostream& dot(std::ostream&) const;
    ///@}

    std::string name() const;
    std::string str_(bool prefix) const;
    std::string str_rep() const { return str_(false); }
    std::string str_aux() const { return str_(true); }

    void aggregate_link(const Expr* up) const { up->agg += this->agg; }
    void aggregate_cut(const Expr* up) const { up->agg -= this->agg; }
    void aggregate() const {
#if 0
        agg = gid;
        for (auto op : ops) agg += op->agg;
#endif
    }

    void splay_link(const Expr* p) const {
        p->left_ = this;
        this->parent_ = p;
    }
    using LinkCutTree<const Expr>::splay;

    World& world;
    size_t gid;
    bool mut;
    Tag tag;
    std::vector<const Expr*> ops;
    uint64_t stuff;
    size_t hash;
    mutable size_t agg = 0;
};

using ExprSet = std::unordered_set<const Expr*, GIDHash<const Expr*>, GIDEq<const Expr*>>;
template<class T>
using ExprMap = std::unordered_map<const Expr*, T, GIDHash<const Expr*>, GIDEq<const Expr*>>;
