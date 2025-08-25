#pragma once
#include <stdint.h>
#include <memory>
#include <cstring>
#include <random>
#include <fmt/core.h>
#include <Tools/Time.h>
#include <Tools/NumberCodec.h>

constexpr uint8_t RTMP_VERSION = 0x03; // RTMP version 3

class Handshake
{
protected:
    static constexpr size_t RANDOM_SIZE = 1528;
    static constexpr size_t HANDSHAKEDATE_SIZE = 4 + 4 + RANDOM_SIZE;
    static constexpr size_t VERSION_SIZE = 1;
    static constexpr size_t C0C1_SIZE = VERSION_SIZE + HANDSHAKEDATE_SIZE;
    static constexpr size_t S0S1S2_SIZE = VERSION_SIZE + HANDSHAKEDATE_SIZE + HANDSHAKEDATE_SIZE;
    static constexpr size_t C2_SIZE = HANDSHAKEDATE_SIZE;
    struct Version
    {
        uint8_t major;
        bool operator==(const Version &other) const
        {
            return major == other.major;
        }
        Version &operator=(const Version &other) = default;
        Version &operator=(const uint8_t version)
        {
            major = version;
            return *this;
        }
        int Decode(const uint8_t *in, size_t len)
        {
            int offset = 0;
            if (len >= VERSION_SIZE)
                major = NumberCodec::DecodeU8(in, len, offset);
            return offset;
        }
        void Encode(std::vector<uint8_t> &buf)
        {
            NumberCodec::EncodeU8(buf, major);
        }
    };
    struct HandshakeData
    {
        uint32_t timestamp;
        uint32_t zero;
        uint8_t random[RANDOM_SIZE];
        bool operator==(const HandshakeData &other) const
        {
            return timestamp == other.timestamp &&
                   zero == other.zero && zero == 0 &&
                   memcmp(random, other.random, RANDOM_SIZE) == 0;
        }
        HandshakeData &operator=(const HandshakeData &other)
        {
            timestamp = other.timestamp;
            zero = other.zero;
            memcpy(random, other.random, RANDOM_SIZE);
            return *this;
        }
        void Build()
        {
            timestamp = Time::GetTimestamp_ms();
            zero = 0;
            std::random_device rd;
            for (int i = 0; i < RANDOM_SIZE; ++i)
            {
                random[i] = rd() % 256;
            }
        }
        int Decode(const uint8_t *in, size_t len)
        {
            int offset = 0;
            if (len >= HANDSHAKEDATE_SIZE)
            {
                timestamp = NumberCodec::DecodeU32(in, len, offset);
                zero = NumberCodec::DecodeU32(in, len, offset);
                memcpy(random, in + offset, RANDOM_SIZE);
                offset += RANDOM_SIZE;
            }
            return offset;
        }
        void Encode(std::vector<uint8_t> &buf)
        {
            NumberCodec::EncodeU32(buf, timestamp);
            NumberCodec::EncodeU32(buf, zero);
            buf.insert(buf.end(), random, random + RANDOM_SIZE);
        }
    };

    struct C0C1
    {
        Version C0;
        HandshakeData C1;
        int Decode(const uint8_t *in, size_t len)
        {
            constexpr size_t size = C0C1_SIZE;
            if (len < size)
                return 0;
            int offset = 0;
            offset += C0.Decode(in + offset, len - offset);
            offset += C1.Decode(in + offset, len - offset);
            return offset;
        }
        std::vector<uint8_t> Encode()
        {
            constexpr size_t size = C0C1_SIZE;
            std::vector<uint8_t> buf;
            buf.reserve(size);
            C0.Encode(buf);
            C1.Encode(buf);
            return buf;
        }
    };
    struct S0S1S2
    {
        Version S0;
        HandshakeData S1;
        HandshakeData S2;
        int Decode(const uint8_t *in, size_t len)
        {
            constexpr size_t size = S0S1S2_SIZE;
            if (len < size)
                return 0;
            int offset = 0;
            offset += S0.Decode(in + offset, len - offset);
            offset += S1.Decode(in + offset, len - offset);
            offset += S2.Decode(in + offset, len - offset);
            return offset;
        }
        std::vector<uint8_t> Encode()
        {
            constexpr size_t size = S0S1S2_SIZE;
            std::vector<uint8_t> buf;
            buf.reserve(size);
            S0.Encode(buf);
            S1.Encode(buf);
            S2.Encode(buf);
            return buf;
        }
    };
    struct C2
    {
        HandshakeData C2;
        int Decode(const uint8_t *in, size_t len)
        {
            constexpr size_t size = C2_SIZE;
            if (len < size)
                return 0;
            int offset = 0;
            offset += C2.Decode(in + offset, len - offset);
            return offset;
        }
        std::vector<uint8_t> Encode()
        {
            constexpr size_t size = C2_SIZE;
            std::vector<uint8_t> buf;
            buf.reserve(size);
            C2.Encode(buf);
            return buf;
        }
    };

protected:
    C0C1 c0c1_;
    S0S1S2 s0s1s2_;
    C2 c2_;
};

class ClientHandshake : public Handshake
{
public:
    using SPtr = std::shared_ptr<ClientHandshake>;

public:
    // RTMP握手状态
    enum class State : uint8_t
    {
        WaitS0S1S2, // Server responds with S0, S1, S2
        Complete,   // Handshake completed
        Error       // Handshake failed
    };

public:
    static SPtr Create()
    {
        return std::make_shared<ClientHandshake>();
    }
    // private:
    ClientHandshake() : handshake_state_(State::WaitS0S1S2) {}
    inline bool IsComplete() const { return handshake_state_ == State::Complete; }
    inline bool IsError() const { return handshake_state_ == State::Error; }

    std::vector<uint8_t> BuildC0C1()
    {
        c0c1_.C0 = RTMP_VERSION;
        c0c1_.C1.Build();
        return c0c1_.Encode();
    }
    std::vector<uint8_t> BuildC2()
    {
        c2_.C2 = s0s1s2_.S1;
        return c2_.Encode();
    }
    int Parse(uint8_t *in, size_t len)
    {
        int offset = 0;
        int ret = 0;
        switch (handshake_state_)
        {
        case State::WaitS0S1S2:
            if (S0S1S2_SIZE == s0s1s2_.Decode(in, len) &&
                c0c1_.C0 == s0s1s2_.S0 && c0c1_.C1 == s0s1s2_.S2)
            {
                handshake_state_ = State::Complete;
                offset = S0S1S2_SIZE;
            }
            else
            {
                handshake_state_ = State::Error;
            }
            break;
        default:
            fmt::print("client error state\n");
            handshake_state_ = State::Error;
        }
        return offset;
    }

private:
    State handshake_state_;
};

class ServerHandshake : public Handshake
{
public:
    using SPtr = std::shared_ptr<ServerHandshake>;

public:
    // RTMP握手状态
    enum class State : uint8_t
    {
        WaitC0C1, // Client sends C0 and C1
        WaitC2,   // Client sends C2
        Complete, // Handshake completed
        Error     // Handshake failed
    };

public:
    static SPtr Create()
    {
        return std::make_shared<ServerHandshake>();
    }
    // private:
    ServerHandshake() : handshake_state_(State::WaitC0C1) {}
    inline bool IsComplete() const { return handshake_state_ == State::Complete; }
    inline bool IsError() const { return handshake_state_ == State::Error; }

    std::vector<uint8_t> BuildS0S1S2()
    {
        s0s1s2_.S0 = c0c1_.C0;
        s0s1s2_.S1.Build();
        s0s1s2_.S2 = c0c1_.C1;
        return s0s1s2_.Encode();
    }
    int Parse(const uint8_t *in, size_t len)
    {
        int offset = 0;
        switch (handshake_state_)
        {
        case State::WaitC0C1:
            if (C0C1_SIZE == c0c1_.Decode(in, len))
            {
                handshake_state_ = State::WaitC2;
                offset = C0C1_SIZE;
            }
            else
            {
                handshake_state_ = State::Error;
            }
            break;
        case State::WaitC2:
            if (C2_SIZE == c2_.Decode(in, len) &&
                c2_.C2 == s0s1s2_.S1)
            {
                handshake_state_ = State::Complete;
                offset = C2_SIZE;
            }
            else
            {
                handshake_state_ = State::Error;
            }
            break;
        default:
            fmt::print("server error state\n");
            handshake_state_ = State::Error;
        }
        return offset;
    }

private:
    State handshake_state_;
};
