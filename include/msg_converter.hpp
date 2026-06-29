#pragma once

#include <cassert>
#include <cstddef>
#include <slcan.hpp>

namespace slcan::core::msg_converter {

static inline std::uint8_t hex_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 0xA;
    }
    return c - 'a' + 0xA;
}

static inline char int_to_hex(std::uint8_t h) {
    assert(h <= 0xF);
    static constexpr char hex_lookup[] = "0123456789ABCDEF";
    return hex_lookup[h];
}

static inline void hex_str_to_arr(const char * str, std::uint8_t * arr, std::size_t digits) {
    assert(str);
    assert(arr);

    const char * end = str + digits;
    std::size_t i = 0;
    
    if (digits & 1) {
        arr[0] = hex_to_int(str[0]);
        ++str;
        i = 1;
    }

    while (str < end) {
        arr[i] = (hex_to_int(str[0]) << 4) | hex_to_int(str[1]);
        str += 2;
        ++i;
    }
}

static inline void arr_to_hex_str(const std::uint8_t * arr, char * str, std::size_t bytes) {
    assert(arr != nullptr);
    assert(str != nullptr);

    std::size_t i = 0;

    if (i < bytes && arr[i] < 0x10) {
        str[0] = int_to_hex(arr[i] & 0x0F);
        ++str;
        i = 1;
    }

    for (; i < bytes; ++i) {
        str[0] = int_to_hex((arr[i] >> 4) & 0x0F);
        str[1] = int_to_hex(arr[i] & 0x0F);
        str += 2;
    } 
}

static inline std::uint32_t hex_str_to_int(const char * data, std::size_t digits) {
    assert(data);
    assert(digits <= sizeof(std::uint32_t) * 2);
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < digits; ++i) {
        value <<= 4;
        value |= hex_to_int(data[i]);
    }
    return value;
}

static inline void int_to_hex_str(std::uint32_t value, char * str, std::size_t digits) {
    assert(digits <= 8);
    assert(str != nullptr);
    for (std::size_t i = digits; i > 0; --i) {
        str[i - 1] = int_to_hex(value & 0x0F);
        value >>= 4;
    }
}

template <CanMsg::Type T>
struct SlCanContentLayout {
    static constexpr std::size_t ID_LENGTH = T == CanMsg::Type::STD ? 3 : 8;
    static constexpr std::size_t MAX_DATA_LENGTH = 16;
    static constexpr std::size_t CR_LENGTH = 1;

    char command;
    char id[ID_LENGTH];
    char dlc;
    char data[MAX_DATA_LENGTH + CR_LENGTH];

    static constexpr std::size_t data_length(std::size_t msg_length) {
        return msg_length - offsetof(SlCanContentLayout<T>, data) - CR_LENGTH;
    }

    static constexpr std::size_t msg_length(std::size_t data_length) {
        return data_length + offsetof(SlCanContentLayout<T>, data) + CR_LENGTH;
    }

    static constexpr std::size_t cr_index(std::size_t msg_length) {
        return msg_length - 1;
    }
};

template <CanMsg::Type T>
bool try_convert(const SlMsg& sl_msg, CanMsg& can_msg) {
    const SlCanContentLayout<T>* content_layout = reinterpret_cast<const SlCanContentLayout<T>*>(sl_msg.content);

    can_msg.type = T;
    can_msg.id = hex_str_to_int(content_layout->id, SlCanContentLayout<T>::ID_LENGTH);
    can_msg.dlc = hex_to_int(content_layout->dlc);

    if (can_msg.id > CanMsg::MAX_ID<T> || can_msg.dlc > CanMsg::MAX_DLC) {
        return false;
    }

    std::size_t data_length = SlCanContentLayout<T>::data_length(sl_msg.length);
    if (data_length != can_msg.dlc * 2) {
        // Data size must match DLC
        return false;
    }

    hex_str_to_arr(content_layout->data, can_msg.data, data_length);

    return true;
}

template <CanMsg::Type T>
void convert(const CanMsg& can_msg, SlMsg& sl_msg) {
    SlCanContentLayout<T>* content_layout = reinterpret_cast<SlCanContentLayout<T>*>(sl_msg.content);

    content_layout->command = T == CanMsg::Type::STD ? \
        static_cast<char>(SlcanCommand::CAN_STD) :\
        static_cast<char>(SlcanCommand::CAN_EXT);

    int_to_hex_str(can_msg.id, content_layout->id, SlCanContentLayout<T>::ID_LENGTH);

    content_layout->dlc = int_to_hex(can_msg.dlc);

    arr_to_hex_str(can_msg.data, content_layout->data, can_msg.dlc);

    sl_msg.length = SlCanContentLayout<T>::msg_length(can_msg.dlc * 2);
    sl_msg.content[SlCanContentLayout<T>::cr_index(sl_msg.length)] = SlMsg::CR;
}

};
