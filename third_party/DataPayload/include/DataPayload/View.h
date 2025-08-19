#pragma once
#include "DataPayload.h"

class DataPayload::View
{
    friend class DataPayload;
    View(const DataPayload *payload, size_t offset = 0, size_t size = 0)
        : data_((offset < payload->size()) ? payload->data() + offset : nullptr),
          size_((size == 0 || size > payload->size() - offset) ? payload->size() - offset : size),
          version_snapshot_(payload->version_), my_version_(*payload->version_) {}

public:
    ~View() = default;
    bool IsValid() const { return data_ != nullptr && my_version_ == *version_snapshot_; }
    const uint8_t *data() const { return IsValid() ? data_ : nullptr; }
    const size_t size() const { return IsValid() ? size_ : 0; }

    std::vector<uint8_t> to_vector() const { return IsValid() ? std::vector<uint8_t>(data_, data_ + size_) : std::vector<uint8_t>(); }
    std::string to_string() const { return IsValid() ? std::string((char *)data_, size_) : std::string(); }

private:
    const uint8_t *data_;                                        // 视图起始点
    const size_t size_;                                          // 视图长度
    const std::shared_ptr<std::atomic_ullong> version_snapshot_; // 原始数据版本号
    const uint64_t my_version_;                                  // 创建视图时的版本号
};