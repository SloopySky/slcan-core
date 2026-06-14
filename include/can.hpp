#pragma once

#include <array>
#include <cstdint>

namespace slcan::core {

class CanMsg;

class CanInterface {
public:
    enum class State { OPEN, CLOSED };

    virtual State state() const = 0;
    virtual void transmit(const CanMsg&) = 0;
    virtual bool open() = 0;
    virtual bool close() = 0;
    ~CanInterface() = default;
};

class CanMsg {
public:
    static constexpr std::uint32_t MAX_STD_ID = 0x7FF;
    static constexpr std::uint32_t MAX_EXT_ID = 0x1FFFFFFF;
    static constexpr std::uint8_t MAX_DLC = 8;

    enum class Type {STD, EXT};

    CanMsg(Type type = Type::STD, std::uint32_t id = 0xFFFFFFFF, std::initializer_list<std::uint8_t> data = {})
        : type_(type), dlc_(data.size() > MAX_DLC ? MAX_DLC : data.size()) {
        if (type == Type::STD) {
            id_ = id > MAX_STD_ID ? MAX_STD_ID : id;
        } else {
            id_ = id > MAX_EXT_ID ? MAX_EXT_ID : id;
        }

        std::copy(data.begin(), data.begin() + dlc_, data_.begin());
    }

    constexpr Type type() const { return type_; }

    constexpr std::uint32_t id() const { return id_; }

    constexpr std::uint8_t dlc() const { return dlc_; }

    constexpr const std::uint8_t * data() const { return data_.data(); }

private:
    Type type_;
    std::uint32_t id_;
    std::uint8_t dlc_;
    std::array<std::uint8_t, MAX_DLC> data_;
};

};

