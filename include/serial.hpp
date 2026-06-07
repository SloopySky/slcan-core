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

    SlMsg() = default;

    SlMsg(const SlMsg& msg) : length_(msg.length()) {
        std::copy(msg.content_.begin(), msg.content_.begin() + msg.length(), content_.begin());
    }

    SlMsg(std::string_view str) {
        if (str.empty() || str.length() > MAX_LENGTH || str.back() != CR) {
            length_ = 0;
        } else {
            std::copy(str.begin(), str.end(), content_.begin());
            length_ = str.length();
        }
    }

    SlMsg(std::initializer_list<char> list) {
        if (list.size() == 0 || list.size() > MAX_LENGTH || *(list.end() - 1) != CR) {
            length_ = 0;
        } else {
            std::copy(list.begin(), list.end(), content_.begin());
            length_ = list.size();
        }
    }

    SlMsg& operator=(const SlMsg& msg) {
        if (this != &msg) {
            std::copy(msg.content_.begin(), msg.content_.begin() + msg.length(), content_.begin());
            length_ = msg.length();
        }
        return *this;
    }

    constexpr char command() const { return length_ ? content_[0] : BELL; }

    constexpr const char* content() const { return content_.data(); }

    constexpr std::size_t length() const { return length_; }

    constexpr bool empty() const { return length_ == 0; }

    void append(char byte) {
        if (empty()) {
            *this = SlMsg({byte, CR});
        } else if (length_ < MAX_LENGTH) {
            content_[length_ - 1] = byte;
            content_[length_] = CR;
            ++length_;
        }
    }

    friend bool operator==(const SlMsg& l, const SlMsg& r) {
        return l.length() == r.length()
            && std::equal(l.content_.begin(), l.content_.begin() + l.length(), r.content_.begin());
    }

private:
    std::array<char, MAX_LENGTH> content_ {};
    std::size_t length_ { 0 };
};

};

