#pragma once

#include <type_traits>

/// [Link/Cut Tree](https://en.wikipedia.org/wiki/Link/cut_tree) that uses
/// [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to make it intrusive.
template<class T>
class LinkCutTree {
public:
    using This                     = LinkCutTree<T>;
    using S                        = std::remove_const_t<T>;
    static constexpr bool is_const = std::is_const_v<T>;

    /// @name Getters
    ///@{
    const S* splay_parent() const { return parent_ && (parent_->left_ == this || parent_->right_ == this) ? parent_ : nullptr; }
    const S* path_parent() const { return parent_ && (parent_->left_ != this && parent_->right_ != this) ? parent_ : nullptr; }
    const S* left() const { return left_; }
    const S* right() const { return right_; }
    const S*& child(size_t i) const { return i == 0 ? left_ : right_; }
    ///@}

    /// Link `this` tree to the @p up%per one.
    void link(const S* up) const {
        self()->expose();
        up->expose();
        up->parent_ = self();
        assert(!right_);
        right_ = up;
    }

    /// Cut `this` tree from its parent.
    void cut() const {
        expose();
        if (auto& r = right_) r = r->parent_ = nullptr;
    }

    /// Make `this` to root a preferred path while putting `this` to the root of the auxiliary splay tree.
    /// @returns the last valid path_parent.
    const S* expose() const {
        const S* prev = nullptr;
        for (auto curr = self(); curr; prev = curr, curr = curr->parent_) {
            curr->splay();
            assert(!prev || prev->parent_ == curr);
            curr->left_ = prev;
        }
        splay();
        return prev;
    }

    /// Find root.
    const S* root() const {
        expose();
        auto curr = self();
        while (auto r = curr->right_) curr = r;
        curr->splay();
        return curr;
    }

    /// Least Common Ancestor.
    /// @returns `nullptr`, if @p a and @p b are in different trees.
    static const S* lca(const S* a, const S* b) {
        if (a == b) return a;
        if (a->root() != b->root()) return nullptr;
        a->expose();
        return b->expose();
    }

    /// @name non-const variants
    ///@{
    S* splay_parent()         requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->splay_parent()); }
    S* path_parent()          requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->path_parent()); }
    S* left()                 requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->left()); }
    S* right()                requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->right()); }
    S*& child(size_t i)       requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->child(i)); }
    S* root()                 requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->root()); }
    S* expose()               requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->expose()); }
    void link(S* up)          requires (!is_const) { return const_cast<const This*>(this)->link(const_cast<const S*>(up)); }
    static S* lca(S* a, S* b) requires (!is_const) { return const_cast<S*>(lca(const_cast<const S*>(a), const_cast<const S*>(b))); }
    ///@}

private:
    const S* self() const { return static_cast<const S*>(this); }
    S* self() requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->self()); }

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
        auto p  = parent_;
        auto sp = splay_parent();
        auto c  = child(r);
        parent_ = c;

        if (c) {
            auto b = c->child(l);
            child(r) = b;
            if (b) b->parent_ = self();
            c->parent_ = p;
            c->child(l) = self();
        }

        if (!sp) {
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

    mutable const S* parent_ = nullptr; ///< parent or path-parent
    mutable const S* left_   = nullptr; ///< left/deeper/down/leaf-direction
    mutable const S* right_  = nullptr; ///< right/shallower/up/root-direction
};
