#include <isa/bus.hpp>

namespace isa {

template <typename BUS_ATTRIBUTES>
void Bus<BUS_ATTRIBUTES>::Register(Listener* listener) {
    listener_ = listener;
}

template <typename BUS_ATTRIBUTES>
Range const& Bus<BUS_ATTRIBUTES>::GetRange() const {
    return range_;
}

template <typename BUS_ATTRIBUTES>
size_t const& Bus<BUS_ATTRIBUTES>::GetIndex() const {
    return index_;
}

template <typename BUS_ATTRIBUTES>
void Bus<BUS_ATTRIBUTES>::Write(
    Address address, typename Attributes::AddressableUnitType const (&data)[Attributes::CountOfAddressableUnits]) {
    if (not range_.Contains(address)) {
        if (listener_) {
            listener_->OnEvent(*this, address, State::Fault, Events::UnassignedFault);
        }
        return;
    }
    if ((address & Attributes::AlignmentMask) > 0 and listener_) {
        listener_->OnEvent(*this, address, State::Fault, Events::UnalignedFault);
    } else {
        listener_->OnEvent(*this, address, State::Fetch, Events::WriteStarted);
        // find the attach memory this address belongs to
        for (auto& memory : attached_memories_) {
            if (memory->ViewRange().Contains(address)) {
                // copy the data to the memory
                for (size_t i = 0; i < Attributes::CountOfAddressableUnits; i++) {
                    (*memory)[address + i] = data[i];
                }
                break;
            }
        }

        listener_->OnEvent(*this, address, State::Idle, Events::WriteCompleted);
    }
    return;
}

template <typename BUS_ATTRIBUTES>
void Bus<BUS_ATTRIBUTES>::Read(Address address) {
    if (not range_.Contains(address)) {
        if (listener_) {
            listener_->OnEvent(*this, address, State::Fault, Events::UnassignedFault);
        }
        return;
    }
    if ((address & Attributes::AlignmentMask) > 0 and listener_) {
        listener_->OnEvent(*this, address, State::Fault, Events::UnalignedFault);
    } else {
        listener_->OnEvent(*this, address, State::Fetch, Events::ReadStarted);
        // todo read data from active memory
        listener_->OnEvent(*this, address, State::Idle, Events::ReadCompleted);
    }
    return;
}

// Emit symbols for the concrete bus types used by tests/demos.
template class Bus<FlashBusAttributes>;
template class Bus<MemoryBusAttributes>;
template class Bus<PeripheralBusAttributes>;

}  // namespace isa
