#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>

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

/* Test CanMsg */
struct CanMsgTestData {
    CanMsg msg;
    CanMsg::Type expected_type;
    std::uint32_t expected_id;
    std::uint8_t expected_dlc;
    std::initializer_list<std::uint8_t> expected_data;
};

class CanMsgTest : public ::testing::TestWithParam<CanMsgTestData> { };

TEST_P(CanMsgTest, CanMsgClass) {
    const auto& test_data = GetParam();
    const CanMsg& msg = test_data.msg;

    EXPECT_EQ(msg.type(), test_data.expected_type);
    EXPECT_EQ(msg.id(), test_data.expected_id);
    EXPECT_EQ(msg.dlc(), test_data.expected_dlc);
    EXPECT_EQ(std::memcmp(msg.data(), test_data.expected_data.begin(), test_data.expected_dlc), 0);
}

INSTANTIATE_TEST_SUITE_P(
    CanMsgTestCases,
    CanMsgTest,
    ::testing::Values(
        CanMsgTestData{ // Default parameters
            CanMsg(),
            CanMsg::Type::STD, CanMsg::MAX_STD_ID, 0, {}
        },
        CanMsgTestData{ // Correct STD
            CanMsg(CanMsg::Type::STD, 0x123, { 0x01, 0x02, 0x03, 0x04 }),
            CanMsg::Type::STD, 0x123, 4, { 0x01, 0x02, 0x03, 0x04 }
        },
        CanMsgTestData{ // Correct EXT
            CanMsg(CanMsg::Type::EXT, 0x12457, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }),
            CanMsg::Type::EXT, 0x12457, 8, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }
        },
        CanMsgTestData{ // STD incorrect ID
            CanMsg(CanMsg::Type::STD, 0xFFFF, { 0x01, 0x02, 0x03, 0x04 }),
            CanMsg::Type::STD, CanMsg::MAX_STD_ID, 4, { 0x01, 0x02, 0x03, 0x04 }
        },
        CanMsgTestData{ // EXT incorrect ID
            CanMsg(CanMsg::Type::EXT, 0xFFFFFFFF, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }),
            CanMsg::Type::EXT, CanMsg::MAX_EXT_ID, 8, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }
        },
        CanMsgTestData{ // Too many data bytes
            CanMsg(CanMsg::Type::STD, 0x123, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF }),
            CanMsg::Type::STD, 0x123, 8, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }
        }
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

