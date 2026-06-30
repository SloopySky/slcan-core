#pragma once

#include <cstring>
#include <string_view>
#include <cstdint>

namespace slcan::core {

struct SlMsg;

class SerialInterface {
public:
    virtual void transmit(const SlMsg&) = 0;
    ~SerialInterface() = default;
};

struct SlMsg {
    static constexpr char CR = 13;
    static constexpr char BELL = 7;
    static constexpr std::size_t MAX_LENGTH = 27;

    char content[MAX_LENGTH];
    std::size_t length { 0 };

    SlMsg() = default;

    SlMsg(const SlMsg& msg) : length(msg.length) {
        std::memcpy(content, msg.content, length);
    }

    SlMsg(std::string_view str) {
        if (str.empty() || str.length() > MAX_LENGTH || str.back() != CR) {
            length = 0;
        } else {
            length = str.length();
            std::memcpy(content, str.data(), length);
        }
    }

    SlMsg(std::initializer_list<char> list) {
        if (list.size() == 0 || list.size() > MAX_LENGTH || *(list.end() - 1) != CR) {
            length = 0;
        } else {
            length = list.size();
            std::memcpy(content, list.begin(), length);
        }
    }

    SlMsg& operator=(const SlMsg& msg) {
        if (this != &msg) {
            length = msg.length;
            std::memcpy(content, msg.content, length);
        }
        return *this;
    }

    constexpr char command() const { return length ? content[0] : BELL; }

    constexpr bool terminated() const {
        return length > 0 && (content[length - 1] == CR || content[length - 1 == BELL]);
    }

    friend bool operator==(const SlMsg& l, const SlMsg& r) {
        return l.length == r.length
            && std::memcmp(l.content, r.content, l.length) == 0;
    }
};

};
