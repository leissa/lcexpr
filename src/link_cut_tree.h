#pragma once

#include <cstddef>

/// [Link/Cut Tree](https://en.wikipedia.org/wiki/Link/cut_tree) that uses
/// [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to make it intrusive.
template<class S>
class LinkCutTree {
public:
    S* self() { return static_cast<S*>(this); }

    void link(S* up) {
        this->expose();
        up->expose();
        up->parent_ = self();
        assert(!root_);
        root_ = up;
    }

    void cut() {
        expose();
        if (auto& r = root_) {
            r->parent_ = nullptr;
            r          = nullptr;
        }
    }

    void expose() {
        for (S *curr = self(), *prev = (S*)nullptr; curr; prev = curr, curr = curr->parent_) {
            curr->splay();
            assert(!prev || prev->parent_ == curr);
            curr->leaf_ = prev;
        }
        splay();
    }

    S* root() {
        expose();
        auto curr = self();
        while (auto r = curr->root_) curr = r;
        curr->splay();
        return curr;
    }

#if 0
    const Expr* Expr::lca(const Expr* a, const Expr* b) {
        if (a == b) return a;
        if (a->root() != b->root()) return nullptr;
        a->expose();
        b->expose();
        return b;
    }
#endif

private:
    S* parent() { return parent_ && (parent_->leaf_ == this || parent_->root_ == this) ? parent_ : nullptr; }
    S* path_parent() { return parent_ && (parent_->leaf_ != this && parent_->root_ != this) ? parent_ : nullptr; }

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
    void rot() {
        constexpr size_t r = (l + 1) % 2;
        auto p = parent_;
        auto ppp = parent();
        auto c = child(r);
        parent_ = c;

        if (c) {
            auto b = c->child(l);
            child(r) = b;
            if (b) b->parent_ = self();
            c->parent_ = p;
            c->child(l) = self();
        }

        if (!ppp) {
            // this is new root
        } else if (p->child(l) == this) {
            p->child(l) = c;
        } else {
            assert(p->child(r) == this);
            p->child(r) = c;
        }
    }

    void rol() { return rot<0>(); }
    void ror() { return rot<1>(); }

    void splay() {
        while (auto p = parent()) {
            if (auto pp = p->parent()) {
                if (p->leaf_ == this && pp->leaf_ == p) {           // zig-zig
                    pp->ror();
                    p->ror();
                } else if (p->root_ == this && pp->root_ == p) {    // zag-zag
                    pp->rol();
                    p->rol();
                } else if (p->leaf_ == this && pp->root_ == p) {    // zig-zag
                    p->ror();
                    pp->rol();
                } else {                                            // zag-zig
                    assert(p->root_ == this && pp->leaf_ == p);
                    p->rol();
                    pp->ror();
                }
            } else if (p->leaf_ == this) {                          // zig
                p->ror();
            } else {                                                // zag
                assert(p->root_ == this);
                p->rol();
            }
        }
    }

    S*& child(size_t i) { return i == 0 ? leaf_ : root_; }
    mutable S* parent_ = nullptr; ///< parent or path-parent
    mutable S* leaf_   = nullptr; ///< left/deeper/down/leaf-direction
    mutable S* root_   = nullptr; ///< right/shallower/up/root-direction
};
