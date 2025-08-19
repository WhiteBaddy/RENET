#include "DataPayload/iterator.h"

DataPayload::iterator::iterator(uint8_t *ptr) : ptr_(ptr) {}
DataPayload::iterator::reference DataPayload::iterator::operator*() const { return *reinterpret_cast<uint8_t *>(ptr_); }
DataPayload::iterator::pointer DataPayload::iterator::operator->() const { return reinterpret_cast<uint8_t *>(ptr_); }
DataPayload::iterator &DataPayload::iterator::operator++()
{
    ++ptr_;
    return *this;
}
DataPayload::iterator DataPayload::iterator::operator++(int)
{
    iterator tmp = *this;
    ++(*this);
    return tmp;
}
DataPayload::iterator &DataPayload::iterator::operator--()
{
    --ptr_;
    return *this;
}
DataPayload::iterator DataPayload::iterator::operator--(int)
{
    iterator tmp = *this;
    --(*this);
    return tmp;
}
DataPayload::iterator DataPayload::iterator::operator+(difference_type n) const { return iterator(ptr_ + n); }
DataPayload::iterator DataPayload::iterator::operator-(difference_type n) const { return iterator(ptr_ - n); }
DataPayload::iterator::difference_type DataPayload::iterator::operator-(const iterator &other) const { return ptr_ - other.ptr_; }
bool DataPayload::iterator::operator==(const iterator &other) const { return ptr_ == other.ptr_; }
bool DataPayload::iterator::operator!=(const iterator &other) const { return ptr_ != other.ptr_; }