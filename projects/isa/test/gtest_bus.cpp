#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <isa/bus.hpp>

using namespace isa;

using TestingAttributes = MemoryBusAttributes;
using TestBus = Bus<TestingAttributes>;

class MockBusListener : public TestBus::Listener {
public:
    MOCK_METHOD(void, OnData,
                (TestBus&, Address,
                 TestBus::Attributes::AddressableUnitType (&data)[TestBus::Attributes::CountOfAddressableUnits]),
                (override));
    MOCK_METHOD(void, OnEvent, (TestBus&, Address, TestBus::State, TestBus::Events), (override));
};

TEST(BusTest, Range) {
    Range range{0x00000000, 0x0000FFFF};
    ASSERT_TRUE(range.Contains(0x00000000));
    ASSERT_TRUE(range.Contains(0x00000001));
    ASSERT_TRUE(range.Contains(0x0000FFFF));
    ASSERT_FALSE(range.Contains(0x00010000));
}

TEST(BusTest, BusIndex) {
    Range range{0x00000000, 0x0000FFFF};
    TestBus bus{0xDEADU, range};
    auto index = bus.GetIndex();
    EXPECT_EQ(0xDEADU, index);
}

TEST(BusTest, WriteUnassignedFault) {
    Range range{0x00000000, 0x0000FFFF};
    TestBus bus{0xDEADU, range};
    MockBusListener listener;
    bus.Register(&listener);

    TestBus::Attributes::AddressableUnitType data[TestBus::Attributes::CountOfAddressableUnits] = {0xFF};
    EXPECT_CALL(listener,
                OnEvent(testing::Ref(bus), 0x00010000U, TestBus::State::Fault, TestBus::Events::UnassignedFault))
        .Times(1);
    bus.Write(0x00010000U, data);  // Out of range address
    ::testing::Mock::VerifyAndClearExpectations(&listener);
}