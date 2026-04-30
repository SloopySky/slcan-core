#pragma once

#include <array>
#include <string_view>
#include <cstdint>

namespace slcan::core {

class SlMsg;

class SerialInterface {
public:
    virtual void transmit(const SlMsg&) = 0;
    ~SerialInterface() = default;
};

class SlMsg {
public:
    static constexpr char CR = 13;
    static constexpr char BELL = 7;
    static constexpr std::size_t MAX_LENGTH = 27;

    std::array<char, MAX_LENGTH> content{};

    constexpr SlMsg() = default;

    constexpr SlMsg(std::string_view str) {
        for (std::size_t i = 0; i < str.size() && i < MAX_LENGTH; ++i) {
            content[i] = str[i];
        }
    }

    constexpr SlMsg(std::initializer_list<char> list) {
        std::size_t i = 0;
        for (char c : list) {
            if (i >= MAX_LENGTH) {
                break;
            }
            content[i] = c;
            ++i;
        }
    }

    char command() const noexcept { return length() ? content[0] : BELL; }

    std::size_t length() const noexcept {
        for (std::size_t i = 0; i < MAX_LENGTH; ++i) {
            if (content[i] == CR || content[i] == BELL) {
                return i + 1;
            }
        }
        return 0; 
    }

    friend bool operator==(const SlMsg& l, const SlMsg& r) {
        return l.length() == r.length()
            && std::equal(l.content.begin(), l.content.begin() + l.length(), r.content.begin());
    }
};

};

