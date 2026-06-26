#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>

#include "serial.hpp"

using namespace slcan::core;

struct SlMsgTestData {
    SlMsg msg;
    std::size_t expectedLength;
    char expectedCommand;
};

class SlMsgTest : public ::testing::TestWithParam<SlMsgTestData> { };

TEST_P(SlMsgTest, SlMsgLengthAndCommand) {
    const auto& test_data = GetParam();
    const SlMsg& msg = test_data.msg;

    EXPECT_EQ(msg.length, test_data.expectedLength);
    EXPECT_EQ(msg.command(), test_data.expectedCommand);
}

INSTANTIATE_TEST_SUITE_P(
    SlMsgTestCases,
    SlMsgTest,
    ::testing::Values(
        SlMsgTestData{ // Empty SlMsg
            SlMsg(), 0, SlMsg::BELL
        },
        SlMsgTestData{ // Unterminated SlMsg, string_view ctor
            SlMsg("V001"), 0, SlMsg::BELL
        },
        SlMsgTestData{ // Terminated SlMsg, string_view ctor
            SlMsg("V001\r"), 5, 'V'
        },
        SlMsgTestData{ // Unterminated SlMsg, initializer_list ctor
            SlMsg({'V', '0', '0', '1'}), 0, SlMsg::BELL
        },
        SlMsgTestData{ // Terminated SlMsg, initializer_list ctor
            SlMsg({'V', '0', '0', '1', '\r'}), 5, 'V'
        },
        SlMsgTestData{ // Initializer too long, string_view ctor
            SlMsg("V00000000000000000000000000000000000\r"), 0, SlMsg::BELL
        },
        SlMsgTestData{ // Initializer too long, initializer_list ctor
            SlMsg({'V', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '\r'}), 0, SlMsg::BELL
        }
    )
);
