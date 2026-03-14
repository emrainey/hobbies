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
        (void)data;
        listener_->OnEvent(*this, address, State::Idle, Events::WriteCompleted);
    }
    return;
}

template <typename BUS_ATTRIBUTES>
void Bus<BUS_ATTRIBUTES>::Read(Address address) {
    (void)address;
    return;
}

// Emit symbols for the concrete bus types used by tests/demos.
template class Bus<MemoryBusAttributes>;
template class Bus<PeripheralBusAttributes>;

}  // namespace isa
