#pragma once

#include <array>
#include <cstdint>

namespace slcan::core {

struct CanMsg;

class CanInterface {
public:
    enum class State { OPEN, CLOSED };

    virtual State state() const = 0;
    virtual void transmit(const CanMsg&) = 0;
    virtual bool open() = 0;
    virtual bool close() = 0;
    ~CanInterface() = default;
};

struct CanMsg {
    enum class Type { STD, EXT };

    template <Type T>
    static constexpr std::size_t MAX_ID = T == Type::STD ? 0x7FF : 0x1FFFFFFF;

    static constexpr std::uint8_t MAX_DLC = 8;

    Type type;
    std::uint32_t id;
    std::uint8_t dlc;
    std::uint8_t data[MAX_DLC];

    CanMsg() = default;

    CanMsg(Type type, uint32_t id, std::initializer_list<std::uint8_t> data = {})
        : type(type), dlc(data.size() > MAX_DLC ? MAX_DLC : data.size()) {
            uint32_t max_id = type == Type::STD ? MAX_ID<Type::STD> : MAX_ID<Type::EXT>;
            this->id = id > max_id ? max_id : id;
            std::memcpy(this->data, data.begin(), dlc);
    }

    friend bool operator==(const CanMsg& l, const CanMsg& r) {
        return l.type == r.type
            && l.id == r.id
            && l.dlc == r.dlc
            && std::memcmp(l.data, r.data, l.dlc) == 0;
    }
};

};

