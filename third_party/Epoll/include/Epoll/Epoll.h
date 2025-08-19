#pragma once
#include <sys/types.h>
#include <sys/epoll.h>
#include <memory.h>
#include <unistd.h>
#include <cstdint>
#include <cerrno>
#include <vector>
#include <memory>
#include <unordered_map>

constexpr auto EVENT_SIZE = 128;
constexpr auto INVALID_EPOLL = -1;

class EpollData
{
public:
    EpollData() = default;
    EpollData(const EpollData &data) { *this = data; }
    explicit EpollData(void *ptr) { m_data.ptr = ptr; }
    explicit EpollData(int fd) { m_data.fd = fd; }
    explicit EpollData(uint32_t u32) { m_data.u32 = u32; }
    explicit EpollData(uint64_t u64) { m_data.u64 = u64; }

    EpollData &operator=(const EpollData &data)
    {
        if (&data != this)
        {
            m_data = data;
        }
        return *this;
    }
    EpollData &operator=(void *data)
    {
        m_data.ptr = data;
        return *this;
    }
    EpollData &operator=(int data)
    {
        m_data.fd = data;
        return *this;
    }
    EpollData &operator=(uint32_t data)
    {
        m_data.u32 = data;
        return *this;
    }
    EpollData &operator=(uint64_t data)
    {
        m_data.u64 = data;
        return *this;
    }

    operator const epoll_data_t &() const { return m_data; }
    operator const epoll_data_t *() const { return &m_data; }

private:
    epoll_data_t m_data{};
};

struct ISharedPtrBox
{
    virtual ~ISharedPtrBox() = default;
};
using PSharedPtrBox = std::shared_ptr<ISharedPtrBox>;
template <typename T>
struct SharedPtrBox : public ISharedPtrBox
{
    std::shared_ptr<T> ptr;
    SharedPtrBox(const std::shared_ptr<T> &p = nullptr) : ptr(p) {}
    ~SharedPtrBox() override = default;
    SharedPtrBox(const SharedPtrBox<T> &obj) = default;
    SharedPtrBox &operator=(const SharedPtrBox<T> &obj) = default;
    operator const std::shared_ptr<T> &() const { return ptr; }
};

class EpollEvent
{
public:
    EpollEvent(uint32_t events, const EpollData &data) : m_event{events, data} {}
    EpollEvent(const epoll_event &event) : m_event(event) {}
    ~EpollEvent() = default;
    // operator epoll_event& () { return m_event; }
    operator epoll_event *() { return &m_event; }
    operator const epoll_event &() { return m_event; }
    operator const epoll_event *() { return &m_event; }
    static size_t size() { return sizeof(epoll_event); }
    bool inline IsErr() const { return m_event.events & EPOLLERR; }
    bool inline IsIn() const { return m_event.events & EPOLLIN; }
    void *PtrData() const { return m_event.data.ptr; }
    int FdData() const { return m_event.data.fd; }
    uint32_t U32Data() const { return m_event.data.u32; }
    uint64_t U64Data() const { return m_event.data.u64; }
    uint32_t Events() const { return m_event.events; }
    template <typename T>
    std::shared_ptr<T> SPtrData() const
    {
        return (m_event.data.ptr == nullptr)
                   ? nullptr
                   : static_cast<SharedPtrBox<T> *>(m_event.data.ptr)->ptr;
    }

private:
    epoll_event m_event{};
};

class EpollEvents
{
public:
    EpollEvents(size_t size = EVENT_SIZE) : m_events(size) {}
    ~EpollEvents() = default;
    EpollEvent &operator[](ssize_t i) { return *(EpollEvent *)&m_events[i]; }
    operator epoll_event *() { return m_events.data(); }
    size_t inline Size() const { return m_events.size(); }
    void inline Resize(size_t size) { m_events.resize(size); }

public:
    // 迭代器类
    class iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = EpollEvent;
        using difference_type = std::ptrdiff_t;
        using pointer = EpollEvent *;
        using reference = EpollEvent &;

        iterator(epoll_event *ptr) : m_ptr(ptr) {}
        reference operator*() const { return *reinterpret_cast<EpollEvent *>(m_ptr); }
        pointer operator->() const { return reinterpret_cast<EpollEvent *>(m_ptr); }
        iterator &operator++()
        {
            ++m_ptr;
            return *this;
        }
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator &operator--()
        {
            --m_ptr;
            return *this;
        }
        iterator operator--(int)
        {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        iterator operator+(difference_type n) const { return iterator(m_ptr + n); }
        iterator operator-(difference_type n) const { return iterator(m_ptr - n); }
        difference_type operator-(const iterator &other) const { return m_ptr - other.m_ptr; }
        bool operator==(const iterator &other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const iterator &other) const { return m_ptr != other.m_ptr; }

    private:
        epoll_event *m_ptr;
    };

    iterator begin() { return iterator(m_events.data()); }
    iterator end() { return iterator(m_events.data() + m_events.size()); }

private:
    std::vector<epoll_event> m_events;
};

class CEpoll
{
public:
    CEpoll();
    ~CEpoll();
    bool IsValid() const;

private:
    CEpoll(const CEpoll &) = delete;
    CEpoll &operator=(const CEpoll &) = delete;

public:
    int Create();
    ssize_t WaitEvents(EpollEvents &events, int timeout = 10);
    // ssize_t WaitEvents(EpollEvents& events, int timeout = 10, ssize_t count = 0) {
    //	if (!IsValid()) return -1;
    //	if (events.Size() <= 0) return -3;
    //	if (count <= 0 || count > events.Size()) count = events.Size();
    //	int ret = epoll_wait(m_epoll, events, count, timeout);
    //	if (ret == -1) {
    //		if (errno == EINTR || errno == EAGAIN) return 0;
    //		return -2;
    //	}
    //	return ret;
    // }
    int Add(int fd, const EpollData &data = EpollData(), uint32_t events = EPOLLIN);
    template <typename T>
    int Add(int fd, const std::shared_ptr<T> &data, uint32_t events = EPOLLIN)
    {
        if (!IsValid())
            return -1;
        if (data == nullptr)
            return -2;
        PSharedPtrBox box = std::make_shared<SharedPtrBox<T>>(data);
        EpollEvent ev(events, EpollData(box.get()));
        int ret = epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, ev);
        if (ret == -1)
            return -3;
        m_objects[fd] = box;
        return 0;
    }
    int Modify(int fd, uint32_t events, const EpollData &data = EpollData());
    template <typename T>
    int Modify(int fd, uint32_t events, const std::shared_ptr<T> &data)
    {
        if (!IsValid())
            return -1;
        if (data == nullptr)
            return -2;
        if (auto it = m_objects.find(fd); it != m_objects.end())
        {
            EpollEvent ev(events, EpollData(it->second.get()));
            int ret = epoll_ctl(m_epoll, EPOLL_CTL_MOD, fd, ev);
            if (ret == -1)
                return -3;
            return 0;
        }
        return -4;
    }
    int Del(int fd);
    void Close();

    inline operator const int() const;

private:
    int m_epoll;
    std::unordered_map<int, PSharedPtrBox> m_objects;
};
