#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>

#include "can.hpp"

using namespace slcan::core;

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

