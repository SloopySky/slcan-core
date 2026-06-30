#include <gtest/gtest.h>
#include <gmock/gmock.h> 
#include <cstring>

#include "can.hpp"

using namespace slcan::core;

struct CanMsgTestData {
    const char * name;
    CanMsg msg;
    CanMsg::Type type;
    std::uint32_t id;
    std::uint8_t dlc;
    std::initializer_list<std::uint8_t> data;
};

class CanMsgTest : public ::testing::TestWithParam<CanMsgTestData> { };

TEST_P(CanMsgTest, CanMsgClass) {
    const auto& test_data = GetParam();

    EXPECT_EQ(test_data.msg.type, test_data.type);
    EXPECT_EQ(test_data.msg.id, test_data.id);
    EXPECT_EQ(test_data.msg.dlc, test_data.dlc);
    EXPECT_EQ(std::memcmp(test_data.msg.data, test_data.data.begin(), test_data.dlc), 0);
}

INSTANTIATE_TEST_SUITE_P(
    CanMsgTestCases,
    CanMsgTest,
    ::testing::Values(
        CanMsgTestData{
            "No_data",
            CanMsg(CanMsg::Type::STD, 0x123),
            CanMsg::Type::STD, 0x123, 0, {}
        },
        CanMsgTestData{ 
            "8_data_bytes",
            CanMsg(CanMsg::Type::STD, 0x123, {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90}),
            CanMsg::Type::STD, 0x123, 8, {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90}
        },
        CanMsgTestData{
            "Too_many_data_bytes",
            CanMsg(CanMsg::Type::STD, 0x123, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF }),
            CanMsg::Type::STD, 0x123, 8, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }
        },
        CanMsgTestData{
            "STD_incorrect_ID",
            CanMsg(CanMsg::Type::STD, 0xFFFF, { 0x01, 0x02, 0x03, 0x04 }),
            CanMsg::Type::STD, CanMsg::MAX_ID<CanMsg::Type::STD>, 4, { 0x01, 0x02, 0x03, 0x04 }
        },
        CanMsgTestData{
            "EXT_incorrect_ID",
            CanMsg(CanMsg::Type::EXT, 0xFFFFFFFF, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }),
            CanMsg::Type::EXT, CanMsg::MAX_ID<CanMsg::Type::EXT>, 8, { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xBA, 0xBB }
        }
    ),
    [](const testing::TestParamInfo<CanMsgTestData>& info) {
        return std::string(info.param.name);
    }
);
