#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>

#include "serial.hpp"

using namespace slcan::core;

struct SlMsgTestData {
    const char * name;
    SlMsg msg;
    std::size_t length;
    char command;
};

class SlMsgTest : public ::testing::TestWithParam<SlMsgTestData> { };

TEST_P(SlMsgTest, SlMsgLengthAndCommand) {
    const auto& test_data = GetParam();
    const SlMsg& msg = test_data.msg;

    EXPECT_EQ(msg.length, test_data.length);
    EXPECT_EQ(msg.command(), test_data.command);
}

INSTANTIATE_TEST_SUITE_P(
    SlMsgTestCases,
    SlMsgTest,
    ::testing::Values(
        SlMsgTestData{
            "Empty_SlMsg", SlMsg(), 0, SlMsg::BELL
        },
        SlMsgTestData{
            "Unterminated_SlMsg_string_view_ctor", SlMsg("V001"), 0, SlMsg::BELL
        },
        SlMsgTestData{
            "Terminated_SlMsg_string_view_ctor", SlMsg("V001\r"), 5, 'V'
        },
        SlMsgTestData{
            "Unterminated_SlMsg_initializer_list_ctor", SlMsg({'V', '0', '0', '1'}), 0, SlMsg::BELL
        },
        SlMsgTestData{
            "Terminated_SlMsg_initializer_list_ctor", SlMsg({'V', '0', '0', '1', '\r'}), 5, 'V'
        },
        SlMsgTestData{
            "Initializer_too_long_string_view_ctor", SlMsg("V00000000000000000000000000000000000\r"), 0, SlMsg::BELL
        },
        SlMsgTestData{
            "Initializer_too_long_initializer_list_ctor", SlMsg({'V', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '\r'}), 0, SlMsg::BELL
        }
    ),
    [](const testing::TestParamInfo<SlMsgTestData>& info) {
        return std::string(info.param.name);
    }
);
