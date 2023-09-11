#pragma once

#include <unordered_set>

#include "expr.h"

struct World {
    struct Hash {
        Hash() {}
        size_t operator()(const Expr* expr) const { return expr->hash; }
    };

    struct Eq {
        Eq() {}
        bool operator()(const Expr* e1, const Expr* e2) const { return Expr::equal(e1, e2); }
    };

    ~World() {
        for (auto expr : set) delete expr;
    }

    size_t next_gid() { return gid++; }

    const Expr* lit(uint64_t u) { return put(new Expr(*this, Tag::Lit, {}, u)); }
    const Expr* id(char c) { return put(new Expr(*this, Tag::Id, {}, uint64_t(c))); }

    const Expr* minus(const Expr* a) {
        auto ops = std::array<const Expr*, 1>{a};
        return put(new Expr(*this, Tag::Minus, ops));
    }

    const Expr* add(const Expr* a, const Expr* b) {
        if ((a->tag != Tag::Lit && b->tag == Tag::Lit) || a->gid > b->gid) std::swap(a, b);

        if (a->tag == Tag::Lit) {
            if (a->stuff == 0) return b;
            if (b->tag == Tag::Lit) return lit(a->stuff + b->stuff);
        }
        auto ops = std::array<const Expr*, 2>{a, b};
        return put(new Expr(*this, Tag::Add, ops));
    }
    const Expr* sub(const Expr* a, const Expr* b) {
        auto ops = std::array<const Expr*, 2>{a, b};
        return put(new Expr(*this, Tag::Sub, ops));
    }
    const Expr* eq(const Expr* a, const Expr* b) {
        auto ops = std::array<const Expr*, 2>{a, b};
        return put(new Expr(*this, Tag::Eq, ops));
    }

    const Expr* select(const Expr* cond, const Expr* t, const Expr* f) {
        auto ops = std::array<const Expr*, 3>{cond, t, f};
        return put(new Expr(*this, Tag::Select, ops));
    }

    const Expr* put(const Expr* expr) {
        auto [i, ins] = set.emplace(expr);
        if (ins) return expr;
        --gid;
        delete expr;
        return *i;
    }

    size_t gid = 0;
    std::unordered_set<const Expr*, Hash, Eq> set;
};

