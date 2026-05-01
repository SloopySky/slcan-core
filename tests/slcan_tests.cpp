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
};

class SlMsgTest : public ::testing::TestWithParam<SlMsgTestData> { };

TEST_P(SlMsgTest, SlMsgLengthAndCommand) {
    const auto& test_data = GetParam();
    const SlMsg& msg = test_data.msg;

    EXPECT_EQ(msg.length(), test_data.expectedLength);
    EXPECT_EQ(msg.command(), test_data.expectedCommand);
}

INSTANTIATE_TEST_SUITE_P(
    SlMsgTestCases,
    SlMsgTest,
    ::testing::Values(
        SlMsgTestData{SlMsg{}, 0, SlMsg::BELL}, // Empty SlMsg
        SlMsgTestData{SlMsg{"V001"}, 0, SlMsg::BELL}, // Unterminated SlMsg
        SlMsgTestData{SlMsg{"V001\r"}, 5, 'V'} // Terminated SlMsg
    )
);


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
        SlcanRequestsTestData{SlMsg{{0}}, SlMsg{{SlMsg::BELL}}}, // Empty request
        SlcanRequestsTestData{SlMsg{"V"}, SlMsg{{SlMsg::BELL}}}, // Unterminated request
        SlcanRequestsTestData{SlMsg{"X"}, SlMsg{{SlMsg::BELL}}}, // Unsupported request
        SlcanRequestsTestData{SlMsg{{SlMsg::CR}}, SlMsg{{SlMsg::CR}}}, // CR command request
        SlcanRequestsTestData{ // V command request
            SlMsg{"V\r"},
            SlMsg{{'V', VERSION[0], VERSION[1], VERSION[2], VERSION[3], SlMsg::CR}},
        },
        SlcanRequestsTestData{ // N command request
            SlMsg{"N\r"},
            SlMsg{{'N', SERIAL_NUMBER[0], SERIAL_NUMBER[1], SERIAL_NUMBER[2], SERIAL_NUMBER[3], SlMsg::CR}}
        }
    )
);

