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
    agg = gid;
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
    , hash(gid) {
    agg = gid;
}

bool Expr::equal(const Expr* e1, const Expr* e2) {
    if (e1->mut || e2->mut) return e1 == e2;

    bool res = e1->tag == e2->tag && e1->stuff == e2->stuff && e1->ops.size() == e2->ops.size();
    size_t n = e1->ops.size();
    for (size_t i = 0; i != n && res; ++i) res &= e1->ops[i]->gid == e2->ops[i]->gid;
    return res;
}

std::string Expr::name() const {
    if (tag == Tag::Lit) return std::to_string(stuff);
    if (tag == Tag::Id) return std::string(1, (char)stuff);
    return tag2str(tag);
}

std::string Expr::str() const { return  "\""s  + std::to_string(gid) + ": " + name() + " - " + std::to_string(agg)+ "\""s ; }
std::string Expr::str2() const { return "\"_"s + std::to_string(gid) + ": " + name() + " - " + std::to_string(agg)+ "\""s ; }

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
            o << '\t' << expr->str() << " -> " << op->str() << "[color=black];" << std::endl;
            enqueue(op);
        }
    }

    done.clear();
    enqueue(this);

    while (!q.empty()) {
        auto expr = q.front();
        q.pop();

        if (auto p = expr->splay_parent()) o << '\t' << expr->str2() << " -> " << p->str2() << "[style=dashed];" << std::endl;
        if (auto p = expr->path_parent()) o << '\t' << expr->str2() << " -> " << p->str2() << "[style=dashed,color=gray];" << std::endl;
        if (auto l = expr->left()) o << '\t' << expr->str2() << " -> " << l->str2() << "[color=green];" << std::endl;
        if (auto r = expr->right()) o << '\t' << expr->str2() << " -> " << r->str2() << "[color=red];" << std::endl;

        for (auto op : expr->ops) enqueue(op);
    }

    return o << "}" << std::endl;;
}
