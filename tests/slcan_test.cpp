#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>
#include <optional>

#include "slcan.hpp"

using namespace slcan::core;

using ::testing::StrictMock;
using ::testing::Mock;

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

struct SlcanRequestsTestData {
    const char * name;
    SlMsg request;
    SlMsg response;
    std::optional<CanMsg> can_msg;
};

class SlcanClosedRequestsTest : public ::testing::TestWithParam<SlcanRequestsTestData> {
protected:
    SlcanClosedRequestsTest() : slcan(serial, can) { }

    MockSerial serial;
    StrictMock<MockCan> can;

    Slcan slcan;

    const SlMsg open_request = SlMsg("O\r");
    const SlMsg close_request = SlMsg("C\r");
};

TEST_F(SlcanClosedRequestsTest, InitialStateClosed) {
    EXPECT_EQ(can.state(), CanInterface::State::CLOSED);
}

TEST_F(SlcanClosedRequestsTest, CloseRequestNotSupportedInClosed) {
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::BELL})));

    slcan.processSlRxMsg(close_request);

    EXPECT_EQ(can.state(), CanInterface::State::CLOSED);
}

TEST_F(SlcanClosedRequestsTest, OpenRequestTransition) {
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::CR})));

    slcan.processSlRxMsg(open_request);

    EXPECT_EQ(can.state(), CanInterface::State::OPEN);
}

TEST_P(SlcanClosedRequestsTest, SlcanClosedRequests) {
    const auto& test_data = GetParam();
 
    EXPECT_CALL(serial, transmit(test_data.response)).Times(1);

    slcan.processSlRxMsg(test_data.request);
}

INSTANTIATE_TEST_SUITE_P(
    SlcanRequestsTestCases,
    SlcanClosedRequestsTest,
    ::testing::Values(
        SlcanRequestsTestData{
            "Unsupported_request",
            SlMsg("X\r"),
            SlMsg({SlMsg::BELL}),
        },
        SlcanRequestsTestData{
            "CR_command_request",
            SlMsg({SlMsg::CR}),
            SlMsg({SlMsg::CR}),
        },
        SlcanRequestsTestData{
            "V_command_request",
            SlMsg("V\r"),
            SlMsg({'V', VERSION[0], VERSION[1], VERSION[2], VERSION[3], SlMsg::CR}),
        },
        SlcanRequestsTestData{
            "N_command_request",
            SlMsg("N\r"),
            SlMsg({'N', SERIAL_NUMBER[0], SERIAL_NUMBER[1], SERIAL_NUMBER[2], SERIAL_NUMBER[3], SlMsg::CR}),
        },
        SlcanRequestsTestData{
            "Std_command_request_not_supported",
            SlMsg("t1232ABCD\r"),
            SlMsg({SlMsg::BELL}),
        },
        SlcanRequestsTestData{
            "Ext_command_request_not_supported",
            SlMsg("T1234ABCD2ABCD\r"),
            SlMsg({SlMsg::BELL}),
        }
    ),
    [](const testing::TestParamInfo<SlcanRequestsTestData>& info) {
        return std::string(info.param.name);
    }
);

class SlcanOpenRequestsTest : public SlcanClosedRequestsTest {
protected:
    void SetUp() override {
        EXPECT_CALL(serial, transmit(SlMsg({SlMsg::CR})));
        slcan.processSlRxMsg(open_request);
    }
};

TEST_F(SlcanOpenRequestsTest, InitialStateOpen) {
    EXPECT_EQ(can.state(), CanInterface::State::OPEN);
}

TEST_F(SlcanOpenRequestsTest, OpenRequestNotSupportedInOpen) {
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::BELL})));

    slcan.processSlRxMsg(open_request);

    EXPECT_EQ(can.state(), CanInterface::State::OPEN);
}

TEST_F(SlcanOpenRequestsTest, CloseRequestTransition) {
    EXPECT_CALL(serial, transmit(SlMsg({SlMsg::CR})));

    slcan.processSlRxMsg(close_request);

    EXPECT_EQ(can.state(), CanInterface::State::CLOSED);
}

TEST_P(SlcanOpenRequestsTest, SlcanOpenRequests) {
    const auto& test_data = GetParam();
 
    EXPECT_CALL(serial, transmit(test_data.response)).Times(1);

    slcan.processSlRxMsg(test_data.request);
}

INSTANTIATE_TEST_SUITE_P(
    SlcanRequestsTestCases,
    SlcanOpenRequestsTest,
    ::testing::Values(
        SlcanRequestsTestData{
            "Unsupported_request",
            SlMsg("X\r"),
            SlMsg({SlMsg::BELL}),
        },
        SlcanRequestsTestData{
            "CR_command_request",
            SlMsg({SlMsg::CR}),
            SlMsg({SlMsg::CR}),
        },
        SlcanRequestsTestData{
            "V_command_request",
            SlMsg("V\r"),
            SlMsg({'V', VERSION[0], VERSION[1], VERSION[2], VERSION[3], SlMsg::CR}),
        },
        SlcanRequestsTestData{
            "N_command_request",
            SlMsg("N\r"),
            SlMsg({'N', SERIAL_NUMBER[0], SERIAL_NUMBER[1], SERIAL_NUMBER[2], SERIAL_NUMBER[3], SlMsg::CR}),
        }
    ),
    [](const testing::TestParamInfo<SlcanRequestsTestData>& info) {
        return std::string(info.param.name);
    }
);

class SlcanCanRequestsTest : public SlcanOpenRequestsTest { };

TEST_P(SlcanCanRequestsTest, SlcanCanRequests) {
    const auto& test_data = GetParam();

    EXPECT_CALL(serial, transmit(test_data.response)).Times(1);

    if (test_data.can_msg) {
        EXPECT_CALL(can, transmit(test_data.can_msg.value())).Times(1);
    }

    slcan.processSlRxMsg(test_data.request);
}

INSTANTIATE_TEST_SUITE_P(
    SlcanRequestsTestCases,
    SlcanCanRequestsTest,
    ::testing::Values(
        SlcanRequestsTestData{
            "STD_ID_out_of_range",
            SlMsg("tFFF2ABCD\r"),
            SlMsg({SlMsg::BELL}),
            std::nullopt,
        },
        SlcanRequestsTestData{
            "EXT_ID_out_of_range",
            SlMsg("TFFFFFFFF2ABCD\r"),
            SlMsg({SlMsg::BELL}),
            std::nullopt,
        },
        SlcanRequestsTestData{
            "DLC_out_of_range",
            SlMsg("t1FF9ABCDEF1234567890AB\r"),
            SlMsg({SlMsg::BELL}),
            std::nullopt,
        },
        SlcanRequestsTestData{
            "DLC_and_data_size_not_matching",
            SlMsg("t1FF4ABCDEF123456\r"),
            SlMsg({SlMsg::BELL}),
            std::nullopt,
        },
        SlcanRequestsTestData{
            "Valid_STD",
            SlMsg("t1FF2ABCD\r"),
            SlMsg({static_cast<char>(SlcanResponse::CAN_STD), SlMsg::CR}),
            CanMsg(CanMsg::Type::STD, 0x1FF, {0xAB, 0xCD}),
        },
        SlcanRequestsTestData{
            "Valid_empty_STD",
            SlMsg("t1FF0\r"),
            SlMsg({static_cast<char>(SlcanResponse::CAN_STD), SlMsg::CR}),
            CanMsg(CanMsg::Type::STD, 0x1FF),
        },
        SlcanRequestsTestData{
            "Valid_EXT",
            SlMsg("T1FFFFFFF8ABCDEF1234567890\r"),
            SlMsg({static_cast<char>(SlcanResponse::CAN_EXT), SlMsg::CR}),
            CanMsg(CanMsg::Type::EXT, 0x1FFFFFFF, {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90}),
        }
    ),
    [](const testing::TestParamInfo<SlcanRequestsTestData>& info) {
        return std::string(info.param.name);
    }
);

struct SlcanCanRxTestData {
    const char * name;
    CanMsg can_msg;
    SlMsg sl_msg;
};

class SlcanCanRxTest : public ::testing::TestWithParam<SlcanCanRxTestData> {
protected:
    SlcanCanRxTest() : slcan(serial, can) { }

    MockSerial serial;
    StrictMock<MockCan> can;

    Slcan slcan;
};

TEST_P(SlcanCanRxTest, SlCanRxTest) {
    const auto& test_data = GetParam();

    EXPECT_CALL(serial, transmit(test_data.sl_msg)).Times(1);

    slcan.processCanRxMsg(test_data.can_msg);
}

INSTANTIATE_TEST_SUITE_P(
    SlcanCanRxTestCases,
    SlcanCanRxTest,
    ::testing::Values(
        SlcanCanRxTestData{
            "STD",
            CanMsg(CanMsg::Type::STD, 0x1FF, {0xAB, 0xCD}),
            SlMsg("t1FF2ABCD\r"),
        },
        SlcanCanRxTestData{
            "Empty_STD",
            CanMsg(CanMsg::Type::STD, 0x1FF),
            SlMsg("t1FF0\r"),
        },
        SlcanCanRxTestData{
            "EXT",
            CanMsg(CanMsg::Type::EXT, 0x1FFFFFFF, {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90}),
            SlMsg("T1FFFFFFF8ABCDEF1234567890\r"),
        }
    ),
    [](const testing::TestParamInfo<SlcanCanRxTestData>& info) {
        return std::string(info.param.name);
    }
);
