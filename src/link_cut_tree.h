#pragma once

#include <cstddef>

/// [Link/Cut Tree](https://en.wikipedia.org/wiki/Link/cut_tree) that uses
/// [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to make it intrusive.
template<class S>
class LinkCutTree {
public:
    /// Link `this` tree to the @p up%per one.
    void link(S* up) {
        self()->expose();
        up->expose();
        up->parent_ = self();
        assert(!right_);
        right_ = up;
    }

    /// Cut `this` tree from its parent.
    void cut() {
        expose();
        if (auto& r = right_) r = r->parent_ = nullptr;
    }

    /// Bring `this` to the root of the auxiliary splay tree.
    /// @returns the last valid path_parent.
    S* expose() {
        S* prev = nullptr;
        for (auto curr = self(); curr; prev = curr, curr = curr->parent_) {
            curr->splay();
            assert(!prev || prev->parent_ == curr);
            curr->left_ = prev;
        }
        splay();
        return prev;
    }

    /// Find root.
    S* root() {
        expose();
        auto curr = self();
        while (auto r = curr->right_) curr = r;
        curr->splay();
        return curr;
    }

    /// Least Common Ancestor.
    /// @returns `nullptr`, if @p a and @p b are in different trees.
    static S* lca(S* a, S* b) {
        if (a == b) return a;
        if (a->root() != b->root()) return nullptr;
        a->expose();
        return b->expose();
    }

    const S* root() const { return const_cast<LinkCutTree<S>*>(this)->root(); }
    const S* expose() const { return const_cast<LinkCutTree<S>*>(this)->expose(); }
    static const S* lca(const S* a, const S* b) { return lca(const_cast<S*>(a), const_cast<S*>(b)); }

private:
    S* self() { return static_cast<S*>(this); }
    S* self() const { return const_cast<S*>(static_cast<const S*>(this)); }
    S* splay_parent() const { return parent_ && (parent_->left_ == this || parent_->right_ == this) ? parent_ : nullptr; }
    S* path_parent() const { return parent_ && (parent_->left_ != this && parent_->right_ != this) ? parent_ : nullptr; }
    S*& child(size_t i) const { return i == 0 ? left_ : right_; }
    void rol() const { return rot<0>(); }
    void ror() const { return rot<1>(); }

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
    void rot() const {
        constexpr size_t r = (l + 1) % 2;
        auto p = parent_;
        auto c = child(r);
        parent_ = c;

        if (c) {
            auto b = c->child(l);
            child(r) = b;
            if (b) b->parent_ = self();
            c->parent_ = p;
            c->child(l) = self();
        }

        if (!splay_parent()) {
            // this is new root
        } else if (p->child(l) == this) {
            p->child(l) = c;
        } else {
            assert(p->child(r) == this);
            p->child(r) = c;
        }
    }

    void splay() const {
        while (auto p = splay_parent()) {
            if (auto pp = p->splay_parent()) {
                if (p->left_ == this && pp->left_ == p) {           // zig-zig
                    pp->ror();
                    p->ror();
                } else if (p->right_ == this && pp->right_ == p) {  // zag-zag
                    pp->rol();
                    p->rol();
                } else if (p->left_ == this && pp->right_ == p) {   // zig-zag
                    p->ror();
                    pp->rol();
                } else {                                            // zag-zig
                    assert(p->right_ == this && pp->left_ == p);
                    p->rol();
                    pp->ror();
                }
            } else if (p->left_ == this) {                          // zig
                p->ror();
            } else {                                                // zag
                assert(p->right_ == this);
                p->rol();
            }
        }
    }

    mutable S* parent_ = nullptr; ///< parent or path-parent
    mutable S* left_   = nullptr; ///< left/deeper/down/leaf-direction
    mutable S* right_  = nullptr; ///< right/shallower/up/root-direction
};
