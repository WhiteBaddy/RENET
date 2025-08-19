#pragma once
struct EventTypeConst;
struct EventType
{
    enum EventTypeBit
    {
        NONE_BIT = 0x00,
        IN_BIT = 0x01,
        PRI_BIT = 0x02,
        OUT_BIT = 0x04,
        ERR_BIT = 0x08,
        HUP_BIT = 0x10
    };

protected:
    unsigned int attr;
    constexpr EventType(EventTypeBit bit) : attr(bit) {}

public:
    explicit EventType(unsigned int type) : attr(type) {}
    constexpr EventType(const EventType &type = EventType(NONE_BIT)) : attr(type.attr) {}
    constexpr EventType &operator=(const EventType &type)
    {
        if (this != &type)
        {
            attr = type.attr;
        }
        return *this;
    }
    constexpr EventType &operator+=(const EventTypeConst &type);
    constexpr EventType &operator-=(const EventTypeConst &type);
    friend constexpr EventType &operator+(EventType t1, const EventTypeConst &t2);
    friend constexpr EventType &operator-(EventType t1, const EventTypeConst &t2);

    inline constexpr bool IsIn() const { return attr & IN_BIT; }
    inline constexpr bool IsPri() const { return attr & PRI_BIT; }
    inline constexpr bool IsOut() const { return attr & OUT_BIT; }
    inline constexpr bool IsErr() const { return attr & ERR_BIT; }
    inline constexpr bool IsHup() const { return attr & HUP_BIT; }
    inline constexpr bool IsNone() const { return attr == NONE_BIT; }

    inline constexpr operator unsigned int() const { return attr; }
};

struct EventTypeConst : public EventType
{
private:
    friend constexpr EventType &EventType::operator+=(const EventTypeConst &type);
    friend constexpr EventType &EventType::operator-=(const EventTypeConst &type);
    friend constexpr EventType &operator+(EventType t1, const EventTypeConst &t2);
    friend constexpr EventType &operator-(EventType t1, const EventTypeConst &t2);

    friend struct EventTypeConstInit;

    constexpr EventTypeConst(EventType::EventTypeBit bit = EventType::NONE_BIT) : EventType(bit) {}
    EventTypeConst(const EventTypeConst &) = delete;
    EventTypeConst &operator=(const EventTypeConst &) = delete;
};

struct EventTypeConstInit
{
    constexpr static EventTypeConst make(EventType::EventTypeBit bit)
    {
        return EventTypeConst(bit);
    }
};

inline constexpr EventType &EventType::operator+=(const EventTypeConst &type)
{
    attr |= type.attr;
    return *this;
}

inline constexpr EventType &EventType::operator-=(const EventTypeConst &type)
{
    attr &= ~type.attr;
    return *this;
}

inline constexpr EventType &operator+(EventType t1, const EventTypeConst &t2)
{
    return t1 += t2;
}

inline constexpr EventType &operator-(EventType t1, const EventTypeConst &t2)
{
    return t1 -= t2;
}

namespace EventTypes
{
    inline constexpr EventTypeConst None = EventTypeConstInit::make(EventType::NONE_BIT);
    inline constexpr EventTypeConst In = EventTypeConstInit::make(EventType::IN_BIT);
    inline constexpr EventTypeConst Pri = EventTypeConstInit::make(EventType::PRI_BIT);
    inline constexpr EventTypeConst Out = EventTypeConstInit::make(EventType::OUT_BIT);
    inline constexpr EventTypeConst Err = EventTypeConstInit::make(EventType::ERR_BIT);
    inline constexpr EventTypeConst Hup = EventTypeConstInit::make(EventType::HUP_BIT);
};
