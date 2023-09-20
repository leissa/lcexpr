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

struct Expr : public LinkCutTree<const Expr> {
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
};

using ExprSet = std::unordered_set<const Expr*, GIDHash<const Expr*>, GIDEq<const Expr*>>;
template<class T>
using ExprMap = std::unordered_map<const Expr*, T, GIDHash<const Expr*>, GIDEq<const Expr*>>;
