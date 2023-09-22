#pragma once

#include <type_traits>

/// [Link/Cut Tree](https://en.wikipedia.org/wiki/Link/cut_tree) that uses
/// [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to make it intrusive.
/// We use the following terminology:
/// * **rep** tree is the *represented* tree that we actually care about.
/// * **aux** tree is the *auxiliary* tree that is used to index paths in the *rep* tree;
///     an *aux* tree is implemented as [splay tree](https://en.wikipedia.org/wiki/Splay_tree).
/// * the **l**%eft child of a node in the *aux* tree points to the **l**%eafs of the *rep* tree,
/// * the **r**%ight child of a node in the *aux* tree points to the **r**%oot of the *rep* tree.
///
/// To make a LinkCutTree, simply derive from this class using one of two variants:
/// 1. With this variant, all methods from LinkCutTree are available as non-`const` methods,
/// as all methods may change some pointers in the internal data structure.
/// ```
/// class MyClass : public<MyClass> { /*...*/ };
/// ```
/// 2. Alternatively, you can argue that internal pointers of the LinkCutTree tree are an implementation detail.
/// With this variant, all inherited methods are also available as `const` methods.
/// ```
/// class MyClass : public<const MyClass> { /*...*/ };
/// ```
///
/// @warning As this is an *intrusive* data structure, it's the responsibility of the user to link/cut nodes in the *rep* tree.
/// This class **only** manages the *aux* tree.
/// @note This data structure actually maintains a forest of *rep* and *aux* trees.
/// @sa [Splay Tree](https://hackmd.io/@CharlieChuang/By-UlEPFS#Splay-Tree-Sleator-Tarjan-1983)
/// @sa [Link/Cut Tree](https://hackmd.io/@CharlieChuang/By-UlEPFS#LinkCut-Tree)
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

    /// Link `this` to @p up%per in *aux* tree.
    /// @warning It's the responsibility of the user to also link it in the *rep* tree accordingly.
    void link(const S* up) const {
        up->expose();
        self()->expose();
        up->parent_     = self();
        self()->right_  = up;
        up->aggregate();
    }

    /// Cut `this` from parent in *aux* tree.
    /// @warning It's the responsibility of the user to also cut it in the *rep* tree accordingly.
    void cut() const {
        expose();
        if (right_) {
            right_->aggregate();
            right_->parent_ = nullptr;
            right_          = nullptr;
        }
    }

    /// Make a preferred path from `this` to root while putting `this` at the root of the *aux* tree.
    /// @returns the last valid LinkCutTree::path_parent.
    const S* expose() const {
        const S* prev = nullptr;
        for (auto curr = self(); curr; prev = curr, curr = curr->parent_) {
            curr->splay();
            assert(!prev || prev->parent_ == curr);
            curr->left_ = prev;
            curr->aggregate();
        }
        splay();
        return prev;
    }

    /// Find root of `this` in *rep* tree.
    const S* root() const {
        expose();
        auto curr = self();
        while (auto r = curr->right_) curr = r;
        curr->splay();
        return curr;
    }

    /// Least Common Ancestor of `this` and @p other in the *rep* tree.
    /// @returns `nullptr`, if @p a and @p b are in different trees.
    const S* lca(const S* other) const {
        if (self() == other) return other;
        if (self()->root() != other->root()) return nullptr;
        self()->expose();
        return other->expose();
    }

    /// Aggregate dummy implementations; "override" in @p S (no `virtual` override required due to CRTP).
    void aggregate() const {}
    //void aggregate_sub(const S*) const {}
    //void aggregate_add(const S*) const {}

    // clang-format off
    /// @name Non-Const Variants
    ///@{
    S* splay_parent()   requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->splay_parent()); }
    S* path_parent()    requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->path_parent()); }
    S* left()           requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->left()); }
    S* right()          requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->right()); }
    S*& child(size_t i) requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->child(i)); }
    S* root()           requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->root()); }
    S* expose()         requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->expose()); }
    void link(S* up)    requires (!is_const) { return const_cast<const This*>(this)->link(const_cast<const S*>(up)); }
    S* lca(S* other)    requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->lca(const_cast<const S*>(other))); }
    ///@}
    // clang-format on

protected:
    const S* self() const { return static_cast<const S*>(this); }
    S* self() requires (!is_const) { return const_cast<S*>(const_cast<const This*>(this)->self()); }

    void rol() const { return rot<0>(); }
    void ror() const { return rot<1>(); }

    /**
     * ```
     *  | Left                  | Right                  |
     *  |-----------------------|------------------------|
     *  |   p              p    |       p          p     |
     *  |   |              |    |       |          |     |
     *  |   x              c    |       x          c     |
     *  |  / \     ->     / \   |      / \   ->   / \    |
     *  | a   c          x   d  |     c   a      d   x   |
     *  |    / \        / \     |    / \            / \  |
     *  |   b   d      a   b    |   d   b          b   a |
     *  ```
     */
    template<size_t l>
    void rot() const {
        constexpr size_t r = (l + 1) % 2;

        auto x = self();
        auto p = x->parent_;
        auto c = x->child(r);
        //auto b = c ? c->child(l) : nullptr;
        auto b = c->child(l);

        if (b) b->parent_ = x;

        if (p) {
            if (p->child(l) == x) {
                p->child(l) = c;
            } else if (p->child(r) == x) {
                p->child(r) = c;
            } else {
                /* only path parent */;
            }
        }

        x->parent_  = c;
        x->child(r) = b;

        //if (c) {
            c->parent_  = p;
            c->child(l) = x;
        //}

        //x->aggregate();
        //c->aggregate();
    }

    /// [Splays](https://hackmd.io/@CharlieChuang/By-UlEPFS#Operation1) `this` to the root of its splay tree.
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
