#pragma once
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>

class DataPayload : public std::enable_shared_from_this<DataPayload>
{
public:
    using SPtr = std::shared_ptr<DataPayload>;

public:
    ~DataPayload();
    static SPtr Create(const std::string &str = "");
    static SPtr Create(const std::vector<uint8_t> &vec);
    static SPtr Create(const void *data, size_t size);
    static SPtr Create(std::vector<uint8_t> &&vec);
    static SPtr Create(const SPtr &data, size_t offset = 0, size_t len = 0);

    std::vector<uint8_t> to_vector() const;
    std::string to_string() const;

    const uint8_t *data() const;
    size_t size() const;
    uint8_t at(size_t index) const;

    void push_back(const std::vector<uint8_t> &vec);
    void push_back(const std::string &str);
    void push_back(const SPtr &ptr);
    void pop_front(size_t offset);

    // private:
    DataPayload(const std::string &str);
    DataPayload(const std::vector<uint8_t> &vec);
    DataPayload(const void *data, size_t size);
    DataPayload(std::vector<uint8_t> &&vec);

private:
    void updateVersion();

private:
    std::vector<uint8_t> buffer_;
    std::shared_ptr<std::atomic_ullong> version_;

public:
    class View;
    friend class View;
    View view(size_t offset = 0, size_t size = 0);

    class iterator;
    friend class iterator;
    iterator begin();
    iterator end();
};