#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <vector>

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

using FlashBusAttributes = BusAttributes<32, 8, 1>;
using TightlyCoupledBusAttributes = BusAttributes<32, 8, 1>;
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
    explicit Memory(Range r)
        : range_{r}
        , data_(static_cast<size_t>((r.Size() + sizeof(typename Attributes::AddressableUnitType) - 1U)
                                    / sizeof(typename Attributes::AddressableUnitType))) {
    }

    /// Views the valid range for this memory
    Range const& ViewRange() const {
        return range_;
    }

    /// Accesses the data at the given address, returning the minimum addressable unit set
    typename Attributes::AddressableUnitType& operator[](Address address) {
        size_t index = static_cast<size_t>(address - range_.start) / sizeof(typename Attributes::AddressableUnitType);
        return data_[index];
    }

    /// Read-only access to the data at the given address, returning the const minimum addressable unit set
    typename Attributes::AddressableUnitType const& operator[](Address address) const {
        size_t index = static_cast<size_t>(address - range_.start) / sizeof(typename Attributes::AddressableUnitType);
        return data_[index];
    }

    /// [Debug] the Peek at the Raw Memory Data by index. This is useful for unit tests to validate the address
    /// calculations.
    typename Attributes::AddressableUnitType const& Peek(size_t index) const {
        return data_[index];
    }

protected:
    Range range_;
    std::vector<typename Attributes::AddressableUnitType> data_;
};

using FlashMemory = Memory<FlashBusAttributes>;
using TightlyCoupledMemory = Memory<TightlyCoupledBusAttributes>;
using MainMemory = Memory<MemoryBusAttributes>;
using PeripheralMemory = Memory<PeripheralBusAttributes>;

/// A bus object connects the processor with memory and peripherals
/// @tparam BUS_ATTRIBUTES The bus attributes type defining the bus characteristics
template <typename BUS_ATTRIBUTES>
class Bus {
public:
    using Attributes = BUS_ATTRIBUTES;

    /// Constructs a Bus that responds to the given Range
    constexpr Bus(size_t index, Range r) : index_{index}, listener_{nullptr}, range_{r}, attached_memories_{} {
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

    /// Attaches a memory to a sub range (up to the full range) of the bus.
    /// @return Returns false if the memory range is not contained within the bus range.
    bool Attach(Memory<BUS_ATTRIBUTES>& memory, Range const& memory_range) {
        if (not range_.Contains(memory_range)) {
            return false;
        }
        // ensure no overlap with existing memories
        for (auto const& attached_memory : attached_memories_) {
            if (attached_memory->ViewRange().Contains(memory_range)
                or memory_range.Contains(attached_memory->ViewRange())) {
                return false;
            }
        }
        // all good, attach it
        attached_memories_.push_back(&memory);
        return true;
    }

    /// [DEBUG] Allows a Peek into the bus. Will not cause fault but will return true/false for a valid address or not.
    bool Peek(Address address, typename Attributes::AddressableUnitType& value) const {
        if (not range_.Contains(address)) {
            return false;
        }

        for (auto& memory : attached_memories_) {
            if (not memory->ViewRange().Contains(address)) {
                continue;
            }
            value = (*memory)[address];
            return true;
        }
        return false;
    }

    /// [DEBUG] Allows a Poke into the bus. Will not cause fault but will return true/false for a valid address or not.
    bool Poke(Address address, typename Attributes::AddressableUnitType value) {
        if (not range_.Contains(address)) {
            return false;
        }

        for (auto& memory : attached_memories_) {
            if (not memory->ViewRange().Contains(address)) {
                continue;
            }
            (*memory)[address] = value;
            return true;
        }
        return false;
    }

protected:
    /// The Bus Index used to delineate transactions
    size_t index_;
    /// The registered Listener (only one allowed)
    Listener* listener_{nullptr};
    /// The valid addressable range
    Range range_;
    /// The set of all attached memories on this bus.
    std::vector<Memory<BUS_ATTRIBUTES>*> attached_memories_;
};

/// A Flash Bus can access on a byte boundary
using FlashBus = Bus<FlashBusAttributes>;

/// A Tightly Coupled Memory Bus can access on a word boundary
using TightlyCoupledBus = Bus<TightlyCoupledBusAttributes>;

/// A Main Memory Bus can access on a 64 bit boundary
using MemoryBus = Bus<MemoryBusAttributes>;

/// A peripheral bus can ONLY access data on a 32 bit boundary
using PeripheralBus = Bus<PeripheralBusAttributes>;

}  // namespace isa