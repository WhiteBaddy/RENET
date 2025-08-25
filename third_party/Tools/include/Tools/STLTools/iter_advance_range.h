#pragma once

template <class Container>
struct iter_advance_range
{
    using base_iter = typename Container::iterator;
    Container &c;
    iter_advance_range(Container &container) : c(container) {}

    struct iterator
    {
        Container *pc;
        base_iter cur, nxt;
        iterator(Container &c_, base_iter it) : pc(&c_), cur(it), nxt(it)
        {
            if (nxt != pc->end())
                ++nxt; // 先把“下一位”备好
        }
        operator base_iter() const { return cur; }
        base_iter operator*() const { return cur; }

        iterator &operator++()
        {
            cur = nxt;
            if (nxt != pc->end())
                ++nxt;
            return *this;
        }
        bool operator!=(iterator const &o) const { return cur != o.cur; }

        struct proxy
        {
            iterator *self;
            using ref_t = typename std::iterator_traits<base_iter>::reference; // 元素引用类型

            // 取元素
            ref_t value() const { return *self->cur; }
            // 让它像指针一样用： e->size() / e->first / e->second
            auto *operator->() const { return std::addressof(*self->cur); }
            // 隐式转换为 base_iter，用于需要迭代器的场景
            operator base_iter() const { return *self; }

            // 删当前元素并让外层 ++ 跳过推进
            void erase()
            {
                self->cur = self->pc->erase(self->cur); // 返回下一元素
            }
        };

        proxy operator*() { return proxy{this}; }
    };
    iterator begin() { return iterator{c, c.begin()}; }
    iterator end() { return iterator{c, c.end()}; }
};

// template <class Container>
// iter_advance_range<Container> iter_advance(Container& c) { return { c }; }