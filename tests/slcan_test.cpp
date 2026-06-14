#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>

#include "slcan.hpp"

using namespace slcan::core;

using ::testing::StrictMock;
using ::testing::Mock;

/* Test Slcan requests */
class MockSerial : public SerialInterface {
public:
    MOCK_METHOD(void, transmit, (const SlMsg& msg), (override));
};

class MockCan : public CanInterface {
public:
    MOCK_METHOD(void, transmit, (const CanMsg& msg), (override));

    State state() const { return state_; }

    bool open() override {
        state_ = State::OPEN;
        return true;
    }

    bool close() override {
        state_ = State::CLOSED;
        return true;
    }

private:
    State state_{State::CLOSED};
};

/* Test Slcan state toggling */
TEST(SlcanTest, SlcanStateTest) {
    MockSerial serial;
    StrictMock<MockCan> can;
    Slcan slcan = Slcan(serial, can);

    const SlMsg open_request = SlMsg("O\r");
    const SlMsg close_request = SlMsg("C\r");

    // Initial state CLOSED
    EXPECT_EQ(can.state(), CanInterface::State::CLOSED);

    // Close request not supported in CLOSED state
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::BELL}))).Times(1);
    slcan.processSlRxMsg(close_request);
    EXPECT_EQ(can.state(), CanInterface::State::CLOSED);
    Mock::VerifyAndClearExpectations(&serial);

    // Open request switches the state to OPEN
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::CR}))).Times(1);
    slcan.processSlRxMsg(open_request);
    EXPECT_EQ(can.state(), CanInterface::State::OPEN);
    Mock::VerifyAndClearExpectations(&serial);

    // Open request not supported in OPEN state
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::BELL}))).Times(1);
    slcan.processSlRxMsg(open_request);
    EXPECT_EQ(can.state(), CanInterface::State::OPEN);
    Mock::VerifyAndClearExpectations(&serial);

    // Close request switches the state to CLOSED
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::CR}))).Times(1);
    slcan.processSlRxMsg(close_request);
    EXPECT_EQ(can.state(), CanInterface::State::CLOSED);
    Mock::VerifyAndClearExpectations(&serial);
}

struct SlcanRequestsTestData {
    SlMsg request;
    SlMsg response;
};

class SlcanRequestsTest : public ::testing::TestWithParam<SlcanRequestsTestData> { };

TEST_P(SlcanRequestsTest, SlcanRequests) {
    const auto& test_data = GetParam();
    MockSerial serial;
    StrictMock<MockCan> can;
    Slcan slcan = Slcan(serial, can);

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

