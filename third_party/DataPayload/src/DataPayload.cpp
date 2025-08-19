#include "DataPayload/DataPayload.h"
#include "DataPayload/View.h"
#include "DataPayload/iterator.h"

DataPayload::~DataPayload() { updateVersion(); }
DataPayload::SPtr DataPayload::Create(const std::string &str) { return std::make_shared<DataPayload>(str); }
DataPayload::SPtr DataPayload::Create(const std::vector<uint8_t> &vec) { return std::make_shared<DataPayload>(vec); }
DataPayload::SPtr DataPayload::Create(const void *data, size_t size) { return std::make_shared<DataPayload>(data, size); }
DataPayload::SPtr DataPayload::Create(std::vector<uint8_t> &&vec) { return std::make_shared<DataPayload>(std::move(vec)); }
DataPayload::SPtr DataPayload::Create(const SPtr &data, size_t offset, size_t len)
{
    if (data == nullptr)
        return Create();
    auto begin = (offset < data->buffer_.size()) ? data->buffer_.begin() + offset : data->buffer_.end();
    auto end = (len == 0 || offset + len > data->buffer_.size()) ? data->buffer_.end() : data->buffer_.begin() + offset + len;
    return Create(std::vector<uint8_t>(begin, end));
}

std::vector<uint8_t> DataPayload::to_vector() const { return buffer_; }
std::string DataPayload::to_string() const { return std::string(buffer_.begin(), buffer_.end()); }

const uint8_t *DataPayload::data() const { return buffer_.data(); }
size_t DataPayload::size() const { return buffer_.size(); }
uint8_t DataPayload::at(size_t index) const { return buffer_.at(index); }

void DataPayload::push_back(const std::vector<uint8_t> &vec)
{
    buffer_.insert(buffer_.end(), vec.begin(), vec.end());
    updateVersion();
}
void DataPayload::push_back(const std::string &str)
{
    buffer_.insert(buffer_.end(), str.begin(), str.end());
    updateVersion();
}
void DataPayload::push_back(const SPtr &ptr)
{
    buffer_.insert(buffer_.end(), ptr->buffer_.begin(), ptr->buffer_.end());
    updateVersion();
}
void DataPayload::pop_front(size_t offset)
{
    if (offset <= 0)
        return;
    if (offset >= size())
    {
        buffer_.clear();
    }
    else
    {
        buffer_.assign(buffer_.begin() + offset, buffer_.end());
    }
    updateVersion();
}

DataPayload::DataPayload(const std::string &str)
    : buffer_(str.begin(), str.end()),
      version_(std::make_shared<std::atomic_ullong>(0)) {}
DataPayload::DataPayload(const std::vector<uint8_t> &vec)
    : buffer_(vec),
      version_(std::make_shared<std::atomic_ullong>(0)) {}
DataPayload::DataPayload(const void *data, size_t size)
    : buffer_((const uint8_t *)data, (const uint8_t *)data + size),
      version_(std::make_shared<std::atomic_ullong>(0)) {}
DataPayload::DataPayload(std::vector<uint8_t> &&vec)
    : buffer_(std::move(vec)),
      version_(std::make_shared<std::atomic_ullong>(0)) {}

void DataPayload::updateVersion()
{
    ++(*version_);
}

DataPayload::View DataPayload::view(size_t offset, size_t size)
{
    return View(this, offset, size);
}

DataPayload::iterator DataPayload::begin()
{
    return iterator(buffer_.data());
}
DataPayload::iterator DataPayload::end()
{
    return iterator(buffer_.data() + buffer_.size());
}