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
                OnEvent(testing::Ref(bus), Address{0x00010000U}, TestBus::State::Fault, TestBus::Events::UnassignedFault))
        .Times(1);
    bus.Write(Address{0x00010000U}, data);  // Out of range address
    ::testing::Mock::VerifyAndClearExpectations(&listener);
}

TEST(BusTest, AttachMemoryInitializeAndExerciseFaultCallbacks) {
    constexpr Address kBusStart{0x00001000U};
    constexpr Address kBusEnd{0x0000100FU};
    constexpr Address kAlignedAddress{0x00001000U};
    constexpr Address kUnalignedAddress{0x00001001U};
    constexpr Address kUnassignedAddress{0x00002000U};

    TestBus bus{0xBEEFU, Range{kBusStart, kBusEnd}};
    Memory<TestingAttributes> memory{Range{kAlignedAddress, kAlignedAddress}};
    MockBusListener listener;

    ASSERT_TRUE(bus.Attach(memory, memory.ViewRange()));
    bus.Register(&listener);

    TestBus::Attributes::AddressableUnitType initial_data[TestBus::Attributes::CountOfAddressableUnits] = {};
    for (size_t i = 0; i < TestBus::Attributes::CountOfAddressableUnits; ++i) {
        initial_data[i] = static_cast<TestBus::Attributes::AddressableUnitType>(0xA0U + i);
    }

    {
        testing::InSequence sequence;
        EXPECT_CALL(listener,
                    OnEvent(testing::Ref(bus), kAlignedAddress, TestBus::State::Fetch, TestBus::Events::WriteStarted));
        EXPECT_CALL(listener,
                    OnEvent(testing::Ref(bus), kAlignedAddress, TestBus::State::Idle, TestBus::Events::WriteCompleted));
    }
    bus.Write(kAlignedAddress, initial_data);

    for (size_t i = 0; i < TestBus::Attributes::CountOfAddressableUnits; ++i) {
        EXPECT_EQ(initial_data[i], memory.Peek(i));
    }
    ::testing::Mock::VerifyAndClearExpectations(&listener);

    EXPECT_CALL(listener,
                OnEvent(testing::Ref(bus), kUnalignedAddress, TestBus::State::Fault, TestBus::Events::UnalignedFault))
        .Times(2);
    EXPECT_CALL(listener,
                OnEvent(testing::Ref(bus), kUnassignedAddress, TestBus::State::Fault, TestBus::Events::UnassignedFault))
        .Times(2);

    bus.Write(kUnalignedAddress, initial_data);
    bus.Read(kUnalignedAddress);
    bus.Write(kUnassignedAddress, initial_data);
    bus.Read(kUnassignedAddress);
}

TEST(BusTest, PeekReflectsWhetherAddressIsBackedByAttachedMemory) {
    constexpr Address kBusStart{0x10000000U};
    constexpr Address kBusEnd{0x100000FFU};
    constexpr Address kMemoryStart{0x10000020U};
    constexpr Address kMemoryEnd{0x1000002FU};
    constexpr Address kInsideAddress{0x10000024U};
    constexpr Address kInBusButUnmappedAddress{0x10000010U};
    constexpr Address kOutsideBusAddress{0x20000000U};

    TestBus bus{1U, Range{kBusStart, kBusEnd}};
    Memory<TestingAttributes> memory{Range{kMemoryStart, kMemoryEnd}};

    ASSERT_TRUE(bus.Attach(memory, memory.ViewRange()));

    memory[kInsideAddress] = static_cast<TestBus::Attributes::AddressableUnitType>(0x5AU);

    TestBus::Attributes::AddressableUnitType value = 0U;
    EXPECT_TRUE(bus.Peek(kInsideAddress, value));
    EXPECT_EQ(static_cast<TestBus::Attributes::AddressableUnitType>(0x5AU), value);

    value = 0U;
    EXPECT_FALSE(bus.Peek(kInBusButUnmappedAddress, value));
    EXPECT_FALSE(bus.Peek(kOutsideBusAddress, value));
}