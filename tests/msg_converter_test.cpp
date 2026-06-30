#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "msg_converter.hpp"

using namespace slcan::core;
using namespace slcan::core::msg_converter;

struct HexTestData {
    std::string str;
    std::vector<std::uint8_t> arr;
    std::uint32_t value;
};

class HexTest : public ::testing::TestWithParam<HexTestData> { };

TEST_P(HexTest, HexStrToArrTest) {
    const auto& test_data = GetParam();

    const std::size_t digits = test_data.str.size();
    const std::size_t bytes = test_data.arr.size();

    std::vector<std::uint8_t> arr(bytes);
    hexStrToArr(test_data.str.data(), arr.data(), digits);
    EXPECT_TRUE(std::equal(arr.begin(), arr.end(), test_data.arr.begin()));
}

TEST_P(HexTest, ArrToHexStrTest) {
    const auto& test_data = GetParam();

    const std::size_t digits = test_data.str.size();
    const std::size_t bytes = test_data.arr.size();

    std::vector<char> str(digits);
    arrToHexStr(test_data.arr.data(), str.data(), bytes);
    EXPECT_TRUE(std::equal(str.begin(), str.end(), test_data.str.begin()));
}

TEST_P(HexTest, HexStrToIntTest) {
    const auto& test_data = GetParam();

    const std::size_t digits = test_data.str.size();

    std::uint32_t value = hexStrToInt(test_data.str.data(), digits);
    EXPECT_EQ(value, test_data.value);
}

TEST_P(HexTest, IntToHexStrTest) {
    const auto& test_data = GetParam();

    const std::size_t digits = test_data.str.size();

    std::vector<char> str(digits);
    intToHexStr(test_data.value, str.data(), digits);
    EXPECT_TRUE(std::equal(str.begin(), str.end(), test_data.str.begin()));
}

INSTANTIATE_TEST_SUITE_P(
    HexTestCases,
    HexTest,
    ::testing::Values(
        HexTestData{ "1234", {0x12, 0x34}, 0x1234 },
        HexTestData{ "ABC",  {0x0A, 0xBC}, 0xABC },
        HexTestData{ "102",  {0x01, 0x02}, 0x102 }
    ),
    [](const testing::TestParamInfo<HexTestData>& info) {
        return std::string(info.param.str);
    }
);


struct InvalidSlMsgConvertTestData {
    const char * name;
    CanMsg::Type type;
    SlMsg msg;
};

class InvalidSlMsgConvertTest : public ::testing::TestWithParam<InvalidSlMsgConvertTestData> { };

TEST_P(InvalidSlMsgConvertTest, InvalidSlMsgConvert) {
    const auto& test_data = GetParam();

    CanMsg can_msg;

    bool result = test_data.type == CanMsg::Type::STD ? \
        tryConvert<CanMsg::Type::STD>(test_data.msg, can_msg) : \
        tryConvert<CanMsg::Type::EXT>(test_data.msg, can_msg);

    EXPECT_FALSE(result);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidSlMsgConvertTestCases,
    InvalidSlMsgConvertTest,
    ::testing::Values(
        InvalidSlMsgConvertTestData{ "STD_ID_out_of_range", CanMsg::Type::STD, SlMsg("tFFF2ABCD\r") },
        InvalidSlMsgConvertTestData{ "EXT_ID_out_of_range", CanMsg::Type::EXT, SlMsg("TFFFFFFFF2ABCD\r") },
        InvalidSlMsgConvertTestData{ "DLC_out_of_range", CanMsg::Type::STD, SlMsg("t1FF9ABCDEF1234567890AB\r") },
        InvalidSlMsgConvertTestData{ "DLC_and_data_size_not_matching", CanMsg::Type::STD, SlMsg("t1FF4ABCDEF123456\r") }
    ),
    [](const testing::TestParamInfo<InvalidSlMsgConvertTestData>& info) {
        return std::string(info.param.name);
    }
);


struct MsgConvertTestData {
    const char * name;
    SlMsg sl_msg;
    CanMsg can_msg;
};

class MsgConvertTest : public ::testing::TestWithParam<MsgConvertTestData> { };

TEST_P(MsgConvertTest, SlToCanMsgConvertTest) {
    const auto& test_data = GetParam();

    CanMsg can_msg;

    bool result = test_data.can_msg.type == CanMsg::Type::STD ? \
        tryConvert<CanMsg::Type::STD>(test_data.sl_msg, can_msg) : \
        tryConvert<CanMsg::Type::EXT>(test_data.sl_msg, can_msg);

    EXPECT_TRUE(result);
    EXPECT_EQ(can_msg, test_data.can_msg);
}

TEST_P(MsgConvertTest, CanToSlMsgConvertTest) {
    const auto& test_data = GetParam();

    SlMsg sl_msg;

    if (test_data.can_msg.type == CanMsg::Type::STD) {
        convert<CanMsg::Type::STD>(test_data.can_msg, sl_msg);
    } else {
        convert<CanMsg::Type::EXT>(test_data.can_msg, sl_msg);
    }

    EXPECT_EQ(sl_msg, test_data.sl_msg);
}

INSTANTIATE_TEST_SUITE_P(
    MsgConvertTestCases,
    MsgConvertTest,
    ::testing::Values(
        MsgConvertTestData{
            "t1FF2ABCD",
            SlMsg("t1FF2ABCD\r"),
            CanMsg(CanMsg::Type::STD, 0x1FF, {0xAB, 0xCD})
        },
        MsgConvertTestData{
            "t1FF0",
            SlMsg("t1FF0\r"),
            CanMsg(CanMsg::Type::STD, 0x1FF)
        },
        MsgConvertTestData{
            "T1FFFFFFF8ABCDEF1234567890",
            SlMsg("T1FFFFFFF8ABCDEF1234567890\r"),
            CanMsg(CanMsg::Type::EXT, 0x1FFFFFFF, {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90})
        }
    ),
    [](const testing::TestParamInfo<MsgConvertTestData>& info) {
        return std::string(info.param.name);
    }
);
