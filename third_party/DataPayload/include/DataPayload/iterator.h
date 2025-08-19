#pragma once
#include "DataPayload.h"

class DataPayload::iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = uint8_t;
    using difference_type = std::ptrdiff_t;
    using pointer = uint8_t *;
    using reference = uint8_t &;

    iterator(uint8_t *ptr);
    reference operator*() const;
    pointer operator->() const;
    iterator &operator++();
    iterator operator++(int);
    iterator &operator--();
    iterator operator--(int);
    iterator operator+(difference_type n) const;
    iterator operator-(difference_type n) const;
    difference_type operator-(const iterator &other) const;
    bool operator==(const iterator &other) const;
    bool operator!=(const iterator &other) const;

private:
    uint8_t *ptr_;
};