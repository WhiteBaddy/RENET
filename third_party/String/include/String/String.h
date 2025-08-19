#pragma once

#include <string>
#include <vector>

namespace detail
{
    using StringBase = std::string;
    using StringView = std::string_view;
}

using StrView = detail::StringView;
class String : public detail::StringBase
{
public:
    using detail::StringBase::StringBase;
    using detail::StringBase::operator=;
    using detail::StringBase::operator+=;
    String(size_t size) : detail::StringBase(size, '\0') {}
    String(const detail::StringBase &str) : detail::StringBase(str) {}
    // template <typename... Args>
    // explicit String(StrView fmtstr, Args &&...args)
    //     : detail::StringBase(fmt::format(fmtstr, std::forward<Args>(args)...)) {}

    operator void *() { return (void *)c_str(); }
    operator void *() const { return (void *)c_str(); }
    operator const void *() { return (void *)c_str(); }
    operator const void *() const { return (void *)c_str(); }

    operator char *() { return (char *)c_str(); }
    operator char *() const { return (char *)c_str(); }
    operator const char *() { return (char *)c_str(); }
    operator const char *() const { return c_str(); }

    operator unsigned char *() { return (unsigned char *)c_str(); }
    operator unsigned char *() const { return (unsigned char *)c_str(); }
    operator const unsigned char *() { return (unsigned char *)c_str(); }
    operator const unsigned char *() const { return (unsigned char *)c_str(); }

    std::vector<char> toVectorChar() { return std::vector<char>(begin(), end()); }
    std::vector<uint8_t> toVectorU8() { return std::vector<uint8_t>(begin(), end()); }
    static String toString(const std::vector<char> &vec) { return String(vec.begin(), vec.end()); }
    static String toString(const std::vector<uint8_t> &vec) { return String(vec.begin(), vec.end()); }
};
