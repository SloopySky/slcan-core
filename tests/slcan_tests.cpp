#include <gtest/gtest.h>
#include <gmock/gmock.h> 

#include "slcan.hpp"
#include "serial.hpp"

using namespace slcan::core;

/* Test SlMsg */
struct SlMsgTestData {
    SlMsg msg;
    std::size_t expectedLength;
    char expectedCommand;
    bool expectedEmpty;
};

class SlMsgTest : public ::testing::TestWithParam<SlMsgTestData> { };

TEST_P(SlMsgTest, SlMsgLengthAndCommand) {
    const auto& test_data = GetParam();
    const SlMsg& msg = test_data.msg;

    EXPECT_EQ(msg.length(), test_data.expectedLength);
    EXPECT_EQ(msg.command(), test_data.expectedCommand);
    EXPECT_EQ(msg.empty(), test_data.expectedEmpty);
}

INSTANTIATE_TEST_SUITE_P(
    SlMsgTestCases,
    SlMsgTest,
    ::testing::Values(
        SlMsgTestData{ // Empty SlMsg
            SlMsg(), 0, SlMsg::BELL, true
        },
        SlMsgTestData{ // Unterminated SlMsg, string_view ctor
            SlMsg("V001"), 0, SlMsg::BELL, true
        },
        SlMsgTestData{ // Terminated SlMsg, string_view ctor
            SlMsg("V001\r"), 5, 'V', false
        },
        SlMsgTestData{ // Unterminated SlMsg, initializer_list ctor
            SlMsg({'V', '0', '0', '1'}), 0, SlMsg::BELL, true
        },
        SlMsgTestData{ // Terminated SlMsg, initializer_list ctor
            SlMsg({'V', '0', '0', '1', '\r'}), 5, 'V', false
        },
        SlMsgTestData{ // Initializer too long, string_view ctor
            SlMsg("V00000000000000000000000000000000000\r"), 0, SlMsg::BELL, true
        },
        SlMsgTestData{ // Initializer too long, initializer_list ctor
            SlMsg({'V', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '\r'}), 0, SlMsg::BELL, true
        }
    )
);

/* Test SlMsg append */
TEST(SlMsgTest, SlMsgAppendTest) {
    SlMsg msg;

    msg.append('V');

    EXPECT_EQ(msg.length(), 2);
    EXPECT_EQ(msg.command(), 'V');
    EXPECT_FALSE(msg.empty());
    EXPECT_EQ(msg.content()[0], 'V');
    EXPECT_EQ(msg.content()[1], '\r');

    msg.append('0');

    EXPECT_EQ(msg.length(), 3);
    EXPECT_EQ(msg.command(), 'V');
    EXPECT_FALSE(msg.empty());
    EXPECT_EQ(msg.content()[0], 'V');
    EXPECT_EQ(msg.content()[1], '0');
    EXPECT_EQ(msg.content()[2], '\r');

    for (std::size_t i = 0; i < 30; ++i) {
        msg.append('0');
    }
    EXPECT_EQ(msg.length(), SlMsg::MAX_LENGTH);
    EXPECT_EQ(msg.command(), 'V');
    EXPECT_FALSE(msg.empty());
}


/* Test Slcan requests */
class MockSerial : public SerialInterface {
public:
    MOCK_METHOD(void, transmit, (const SlMsg& msg), (override));
};

struct SlcanRequestsTestData {
    SlMsg request;
    SlMsg response;
};

class SlcanRequestsTest : public ::testing::TestWithParam<SlcanRequestsTestData> { };

TEST_P(SlcanRequestsTest, SlcanRequests) {
    const auto& test_data = GetParam();
    MockSerial serial;
    Slcan slcan = Slcan(serial);

    EXPECT_CALL(serial, transmit(test_data.response)).Times(1);

    slcan.processSlRxMsg(test_data.request);
}

INSTANTIATE_TEST_SUITE_P(
    SlcanRequestsTestCases,
    SlcanRequestsTest,
    ::testing::Values(
        SlcanRequestsTestData{ // Empty request
            SlMsg({0}),
            SlMsg({SlMsg::BELL}),
        },
        SlcanRequestsTestData{ // Unterminated request
            SlMsg("V"),
            SlMsg({SlMsg::BELL}),
        },
        SlcanRequestsTestData{ // Unsupported request
            SlMsg("X"),
            SlMsg({SlMsg::BELL}),
        },
        SlcanRequestsTestData{ // CR command request
            SlMsg({SlMsg::CR}),
            SlMsg({SlMsg::CR}),
        },
        SlcanRequestsTestData{ // V command request
            SlMsg("V\r"),
            SlMsg({'V', VERSION[0], VERSION[1], VERSION[2], VERSION[3], SlMsg::CR}),
        },
        SlcanRequestsTestData{ // N command request
            SlMsg("N\r"),
            SlMsg({'N', SERIAL_NUMBER[0], SERIAL_NUMBER[1], SERIAL_NUMBER[2], SERIAL_NUMBER[3], SlMsg::CR}),
        }
    )
);

