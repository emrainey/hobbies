#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <array>

#include <isa/types.hpp>

namespace isa {

/// An attribute class holds the various attributes shared between Memory and Bus
/// @tparam BUS_WIDTH The width of the bus in bits (e.g., 8, 16, 32, 64)
/// @tparam UNIT_WIDTH The width of the smallest addressable unit in bits (e.g., 8, 16, 32, 64)
/// @tparam COUNT The number of UNITs the bus will transfer in a single operation.
/// @tparam ADDRESS_TYPE The type used to represent addresses on this bus.
template <size_t BUS_WIDTH, size_t UNIT_WIDTH, size_t COUNT>
class BusAttributes {
public:
    /// The number of UNITs that fit in a single BUS transfer
    constexpr static size_t WidthInUnits{BUS_WIDTH / UNIT_WIDTH};

    /// The mask used to determine alignment
    constexpr static Address AlignmentMask{static_cast<Address>(WidthInUnits - 1U)};

    /// The count of the number of addressable units transferred in a single operation
    constexpr static size_t CountOfAddressableUnits{COUNT};

    /// The largest contiguous block of Units this Bus can address
    constexpr static size_t LargestAddressableContiguousUnits{COUNT * WidthInUnits};

    /// The type of the largest addressable unit
    using AddressableUnitType = std::conditional_t<
        WidthInUnits == 1, std::uint8_t,
        std::conditional_t<WidthInUnits == 2, std::uint16_t,
                           std::conditional_t<WidthInUnits == 4, std::uint32_t,
                                              std::conditional_t<WidthInUnits == 8, std::uint64_t, void>>>>;
};

using MemoryBusAttributes = BusAttributes<64, 8, 1>;
using PeripheralBusAttributes = BusAttributes<32, 32, 1>;

/// A memory object connects the processor with memory and peripherals
/// @tparam BUS_ATTRIBUTES The bus attributes type defining the bus characteristics
template <typename BUS_ATTRIBUTES>
class Memory {
public:
    using Attributes = BUS_ATTRIBUTES;

    /// Constructs a Memory that responds to the given Range
    /// @note Does not initialize memory! Client Program in Emulation is responsible for that.
    constexpr Memory(Range r) : range_{r} {
    }

    /// Determines if the given address is contained within the memory range
    constexpr bool Contains(Address address) const {
        return range_.Contains(address);
    }

protected:
    Range range_;
};

using MainMemory = Memory<MemoryBusAttributes>;
using PeripheralMemory = Memory<PeripheralBusAttributes>;

/// A bus object connects the processor with memory and peripherals
/// @tparam BUS_ATTRIBUTES The bus attributes type defining the bus characteristics
template <typename BUS_ATTRIBUTES>
class Bus {
public:
    using Attributes = BUS_ATTRIBUTES;

    /// Constructs a Bus that responds to the given Range
    constexpr Bus(size_t index, Range r) : index_{index}, listener_{nullptr}, range_{r} {
    }

    /// A write operation on the Bus
    /// @param address The address to write to
    /// @param data The data to write
    void Write(Address address,
               typename Attributes::AddressableUnitType const (&data)[Attributes::CountOfAddressableUnits]);

    /// Starts a write operation on the Bus
    /// @param address The address to read from
    /// @warning A Listener must be install to get the results of a Read.
    void Read(Address address);

    enum class State : uint8_t {
        Idle = 0,
        Fetch = 1,
        Fault = 2,
    };

    enum class Input : uint8_t {
        None = 0,
        ReadRequest = 1,
        WriteRequest = 2,
        Reset = 3,
    };

    enum class Events : int8_t {
        UnassignedFault = -2,
        UnalignedFault = -1,
        None = 0,
        WriteStarted = 1,
        WriteCompleted = 2,
        ReadStarted = 3,
        ReadCompleted = 4,
    };

    /// The listener class for Bus events
    class Listener {
    public:
        /// Called when a read completes and the data is available
        virtual void OnData(Bus& bus, Address requested,
                            typename Attributes::AddressableUnitType (&data)[Attributes::CountOfAddressableUnits]) = 0;

        /// Communicates the events to the listeners
        virtual void OnEvent(Bus& bus, Address address, State state, Events event) = 0;

    protected:
        ~Listener() = default;
    };

    /// Registers a Listener to receive Bus events
    void Register(Listener* listener);

    /// The valid range for the bus
    Range const& GetRange() const;

    /// The index given to the bus at construction
    size_t const& GetIndex() const;

protected:
    /// The Bus Index used to delineate transactions
    size_t index_;
    /// The registered Listener (only one allowed)
    Listener* listener_{nullptr};
    /// The valid addressable range
    Range range_;
};

using MemoryBus = Bus<MemoryBusAttributes>;
using PeripheralBus = Bus<PeripheralBusAttributes>;

}  // namespace isa