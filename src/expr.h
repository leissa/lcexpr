#pragma once

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

    static void rot_down(const Expr*);
    static void rot_up(const Expr*);

    struct {
        const Expr* down   = nullptr;
        const Expr* up     = nullptr;
        const Expr* parent = nullptr;
    } mutable lc; // intrusive Link/Cut Tree
};
