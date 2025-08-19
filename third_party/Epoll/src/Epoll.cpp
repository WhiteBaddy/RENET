#include "Epoll/Epoll.h"

CEpoll::CEpoll() : m_epoll(INVALID_EPOLL) {}
CEpoll::~CEpoll() { Close(); }
bool CEpoll::IsValid() const { return m_epoll != INVALID_EPOLL; }

int CEpoll::Create()
{
    if (IsValid())
        return -1;
    m_epoll = epoll_create(1);
    if (!IsValid())
        return -2;
    return 0;
}
ssize_t CEpoll::WaitEvents(EpollEvents &events, int timeout)
{
    if (!IsValid())
        return -1;
    int ret = epoll_wait(m_epoll, events, (int)events.Size(), timeout);
    if (ret == -1)
    {
        if (errno == EINTR || errno == EAGAIN)
            return 0;
        return -2;
    }
    events.Resize(ret);
    return ret;
}
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
int CEpoll::Add(int fd, const EpollData &data, uint32_t events)
{
    if (!IsValid())
        return -1;
    EpollEvent ev(events, data);
    int ret = epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, ev);
    if (ret == -1)
        return -2;
    return 0;
}
// template <typename T>
// int Add(int fd, const std::shared_ptr<T> &data, uint32_t events = EPOLLIN)
// {
//     if (!IsValid())
//         return -1;
//     if (data == nullptr)
//         return -2;
//     PSharedPtrBox box = std::make_shared<SharedPtrBox<T>>(data);
//     EpollEvent ev(events, EpollData(box.get()));
//     int ret = epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, ev);
//     if (ret == -1)
//         return -3;
//     m_objects[fd] = box;
//     return 0;
// }
int CEpoll::Modify(int fd, uint32_t events, const EpollData &data)
{
    if (!IsValid())
        return -1;
    EpollEvent ev(events, data);
    int ret = epoll_ctl(m_epoll, EPOLL_CTL_MOD, fd, ev);
    if (ret == -1)
        return -2;
    return 0;
}
// template <typename T>
// int Modify(int fd, uint32_t events, const std::shared_ptr<T> &data)
// {
//     if (!IsValid())
//         return -1;
//     if (data == nullptr)
//         return -2;
//     if (auto it = m_objects.find(fd); it != m_objects.end())
//     {
//         EpollEvent ev(events, EpollData(it->second.get()));
//         int ret = epoll_ctl(m_epoll, EPOLL_CTL_MOD, fd, ev);
//         if (ret == -1)
//             return -3;
//         return 0;
//     }
//     return -4;
// }
int CEpoll::Del(int fd)
{
    if (!IsValid())
        return -1;
    int ret = epoll_ctl(m_epoll, EPOLL_CTL_DEL, fd, NULL);
    if (ret == -1)
        return -2;
    m_objects.erase(fd);
    return 0;
}
void CEpoll::Close()
{
    if (IsValid())
    {
        int fd = m_epoll;
        m_epoll = INVALID_EPOLL;
        close(fd);
    }
}

inline CEpoll::operator const int() const { return m_epoll; }