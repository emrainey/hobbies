#pragma once

#include <isa/types.hpp>
#include <isa/persistence.hpp>
#include <filesystem>

namespace isa {

/// This implements a hardware cache that can be used for either instruction or data caching. It is a direct-mapped
/// cache with a fixed line size and a fixed number of lines. The cache is organized as an array of cache lines, where
/// each line contains a valid bit, a tag, and a block of data. The cache uses the address to determine the index of the
/// cache line and the tag for comparison. On a cache access, the cache checks if the valid bit is set and if the tag
/// matches the address tag. If both conditions are true, it is a cache hit and the data is returned. Otherwise, it is a
/// cache miss and the cache line is updated with the new data from memory. It is assumed that addresses are byte
/// precise, meaning that the base unit itself can be subaddressed (if not a byte). For example, if the cache is
/// organized with 4 byte words as the addressable unit, then the offset within the cache line will be calculated based
/// on the byte address and the cache will need to handle fetching and writing data at the correct offset within the
/// cache line.
/// @tparam UNIT_TYPE The type of the addressable unit in the cache (e.g. byte, word, etc.)
/// @tparam LINE_COUNT The number of cache lines in the cache. Must be a power of two.
/// @tparam UNITS_PER_LINE The number of addressable units in each cache line. Must be a power of two.
template <typename UNIT_TYPE, size_t LINE_COUNT, size_t UNITS_PER_LINE>
struct Cache {
    constexpr static bool debug = true;  // Set to true to enable debug output for cache updates and evictions.
    using UnitType = UNIT_TYPE;
    constexpr static size_t UnitSizeInBytes = sizeof(UnitType);
    static_assert(LINE_COUNT > 0, "Cache must have at least one line");
    static_assert(UNITS_PER_LINE > 0, "Cache lines must have at least one unit");
    static_assert(is_power_of_two(UNITS_PER_LINE), "Must be a power of two");
    static_assert(is_power_of_two(LINE_COUNT), "Must be a power of two");
    constexpr static size_t LineCount = LINE_COUNT;
    constexpr static size_t UnitsPerLine = UNITS_PER_LINE;

    constexpr static size_t LineSizeInBytes = UnitsPerLine * UnitSizeInBytes;
    constexpr static size_t TotalSize = LineCount * LineSizeInBytes;
    constexpr static size_t IndexBits = log2(LineCount);
    constexpr static size_t OffsetBits = log2(UnitsPerLine);
    constexpr static size_t UnitOffsetBits = log2(UnitSizeInBytes);
    constexpr static size_t TagBits = CountOfDataBits - IndexBits - OffsetBits - UnitOffsetBits;

    using Index = index<IndexBits>;
    using Tag = index<TagBits>;

    /// The Cache Statistics structure keeps track of the number of hits, misses, evictions, invalidations, and updates
    /// that have occurred in the cache. It also provides a method for calculating the hit rate.
    struct Statistics {
        size_t hits{0};
        size_t misses{0};
        size_t evictions{0};
        size_t invalidations{0};
        size_t updates{0};

        double HitRate() const {
            const size_t total_accesses = hits + misses;
            return total_accesses > 0 ? static_cast<double>(hits) / total_accesses : 0.0;
        }

        friend std::ostream& operator<<(std::ostream& os, Statistics const& stats) {
            os << "Hits: " << stats.hits << " Misses: " << stats.misses << " Updates: " << stats.updates
               << " Evictions: " << stats.evictions << " Invalidations: " << stats.invalidations
               << " Hit Rate: " << std::fixed << std::setprecision(2) << (stats.HitRate() * 100.0) << "%";
            return os;
        }
    };

    /// Converts an Address into the index into the Cache Lines
    /// @return The index of the cache line that this address maps to.
    constexpr static Index AddressToIndex(Address address) {
        uint32_t addr = static_cast<uint32_t>(address);
        uint32_t mask = (LineCount - 1);
        return Index{(addr >> (OffsetBits + UnitOffsetBits)) & mask};
    }

    /// Converts an Address into the tag for comparison in the Cache Lines
    /// @return The parts of the address which are used for differentiating between different cache lines that map to
    /// the same index.
    constexpr static Tag AddressToTag(Address address) {
        return Tag{address() >> (OffsetBits + IndexBits + UnitOffsetBits)};
    }

    /// Converts an Address into the offset within the cache line for accessing the correct unit in the cache line's
    /// data array.
    constexpr static uint32_t AddressToOffset(Address address) {
        return (address() >> UnitOffsetBits) & (UnitsPerLine - 1);
    }

    /// The cache line structure contains the information regarding a subset of the memory that is cached.
    struct Line {
        using Data = std::array<UnitType, UnitsPerLine>;
        bool valid_{false};  ///< Valid bit indicating if the cache line contains valid data
        bool dirty_{false};  ///< Dirty bit indicating if the cache line has been modified since it was loaded from
                             ///< memory and needs to be written back to memory on eviction
        Tag tag_;            ///< The tag for the cache line, used for comparison on cache access
        Data data_;          ///< The data stored in the cache line

        constexpr Line() : valid_{false}, dirty_{false}, tag_{0}, data_{} {
        }

        Address GetAddress() const {
            // This function reconstructs the address from the cache line's tag and assumed index of 0.
            // In a real implementation, we would also need to consider the offset within the cache line, but for
            // simplicity, we will assume that the offset is always zero.
            uint32_t tmp = static_cast<uint32_t>(tag_);
            return Address{(tmp << (IndexBits + OffsetBits + UnitOffsetBits))};
        }

        friend std::ostream& operator<<(std::ostream& os, Line const& line) {
            os << "Valid: " << line.valid_ << " Dirty: " << line.dirty_ << " Tag: " << line.tag_ << " Data: [";
            for (size_t i = 0; i < UnitsPerLine; ++i) {
                os << std::hex << line.data_[i];
                if (i < UnitsPerLine - 1) {
                    os << ", ";
                }
            }
            os << "]";
            return os;
        }
    };

    /// A simple listener interface for monitoring cache events such as hits, misses, and evictions. In a real
    /// implementation, this could be used for performance monitoring, debugging, etc.
    class Listener {
    public:
        /// Called when a cache hit occurs, providing the address that caused the hit.
        virtual void OnCacheHit(Address address) = 0;

        /// Called when a cache miss occurs, providing the address that caused the miss. Receivers of the callback
        /// should instigate a memory read to fetch the data for the cache line corresponding to the address and then
        /// write it to the cache using the Update method.
        virtual void OnCacheMiss(Address address) = 0;

        /// Called when evicting a cache line, providing the address being evicted and the contents of the cache line.
        /// In a real implementation, this could be used for writing back dirty lines to memory, logging evictions, etc.
        virtual void OnCacheEvict(Address address, Line const& line) = 0;

        /// Called when a cache invalidate occurs, providing the address being invalidated. This can be used for
        /// maintaining cache coherence in a multi-core system, for example.
        virtual void OnCacheInvalidate(Address address) = 0;

        /// Called when a cache write occurs, providing the address being written to
        virtual void OnCacheUpdate(Address address) = 0;
    };

    constexpr Cache() : Cache{nullptr} {
    }

    constexpr Cache(Listener* listener) : listener_{listener}, lines_{} {
    }

    constexpr Cache(Cache const&) = delete;
    constexpr Cache(Cache&&) = delete;
    Cache& operator=(Cache const&) = delete;
    Cache& operator=(Cache&&) = delete;

    /// Fetches the cache line corresponding to the address and returns true if it was a cache hit, false if it was a
    /// cache miss. On a cache hit, the line parameter will be populated with the data from the cache line. On a cache
    /// miss, the line parameter will be invalid and should not be used.
    bool Fetch(Address address, Line& line) {
        const Index index = AddressToIndex(address);
        const Tag tag = AddressToTag(address);
        if (lines_[index].valid_ and lines_[index].tag_ == tag) {
            line = lines_[index];
            Hit(address);
            return true;  // Cache hit
        } else {
            line.valid_ = false;  // Cache miss
            Miss(address);
            return false;
        }
    }

    /// Fetches a portion of the cache line corresponding to the address.
    /// @param address The address being accessed, which is used to determine the cache line index and tag.
    /// @param data The buffer to store the fetched data, which should be an array of UnitType with at least 'length'
    /// elements.
    /// @param length The number of units to fetch, which should be less than or equal to UnitsPerLine - offset.
    /// @returns true if it was a cache hit, false if it was a cache miss.
    bool Fetch(Address address, UnitType* data, size_t length) {
        const Index index = AddressToIndex(address);
        const Tag tag = AddressToTag(address);
        const uint32_t offset = AddressToOffset(address);
        if (lines_[index].valid_ and lines_[index].tag_ == tag) {
            std::copy(lines_[index].data_.begin() + offset, lines_[index].data_.begin() + offset + length, data);
            Hit(address);
            return true;  // Cache hit
        } else {
            std::fill(data, data + length, UnitType{0});  // Cache miss
            Miss(address);
            return false;
        }
    }

    /// Updates the entire cache line corresponding to the address with the provided data. This is used for both filling
    /// the cache line on a miss and for writing to the cache on a hit. If the cache line being updates as a whole to is
    /// dirty and valid but has a different tag, it will be evicted before writing the new data.
    bool Update(Address address, UnitType const* data) {
        const Index index = AddressToIndex(address);
        const Tag tag = AddressToTag(address);
        if constexpr (debug) {
            std::cout << "Updating cache line from address " << address << " at index " << index << " with tag " << tag
                      << std::endl;
        }

        Maintenance(index, tag);

        lines_[index].valid_ = true;
        lines_[index].dirty_ = false;  // The line is not dirty after we write new data to it since it is now clean with
                                       // the new data, even if it was dirty before the update.
        lines_[index].tag_ = tag;
        std::copy(data, data + UnitsPerLine, lines_[index].data_.begin());
        statistics_.updates++;
        if (listener_) {
            listener_->OnCacheUpdate(address);
        }
        return true;  // Update is always successful
    }

    /// Updates a portion of the cache line corresponding to the address with the provided data. The offset specifies
    /// the starting unit within the cache line to write to, and the length specifies how many units to write. This is
    /// used for writing to a portion of the cache line on a hit.
    /// @param address The address being written to, which is used to determine the cache line index and tag.
    /// @param data The data to write to the cache line, which should be an array of UnitType with at least 'length'
    /// elements.
    /// @param length The number of units to write, which should be less than or equal to UnitsPerLine - offset.
    bool Update(Address address, UnitType const* data, size_t length) {
        const Index index = AddressToIndex(address);
        const Tag tag = AddressToTag(address);
        // the offset will be calculated from the Address
        const uint32_t offset = AddressToOffset(address);
        if constexpr (debug) {
            std::cout << "Updating cache line from address " << address << " at index " << index << " with tag " << tag
                      << " and offset " << offset << " for length " << length << std::endl;
        }
        if (lines_[index].valid_ and lines_[index].dirty_ and lines_[index].tag_ != tag) {
            Evict(index);
            Miss(index);
        }
        lines_[index].valid_ = true;
        lines_[index].dirty_ = true;
        lines_[index].tag_ = tag;
        std::copy(data, data + length, lines_[index].data_.begin() + offset);
        statistics_.updates++;
        if (listener_) {
            listener_->OnCacheUpdate(address);
        }
        return true;  // Update is always successful
    }

    /// Evicts the cache line corresponding to the given address, writing it back to memory if it is dirty and valid,
    /// and then invalidating the cache line.
    void Evict(Address address) {
        Evict(AddressToIndex(address));
    }

    /// Invalidates the cache line corresponding to the given address, marking it as invalid and not dirty, and clearing
    /// its tag and data. This is used for maintaining cache coherence in a multi-core system, for example.
    void Invalidate(Address address) {
        Invalidate(AddressToIndex(address));
    }

    /// Clears all cache lines and resets the cache statistics. This is used for resetting the cache to a clean state,
    void Clean() {
        for (uint32_t i = 0; i < LineCount; ++i) {
            Clear(Index{i});
        }
        statistics_ = Statistics{};
    }

    /// @return True if the cache line corresponding to the address is valid and has a matching tag, indicating a cache
    /// hit.
    bool HasHit(Address address) const {
        const Index index = AddressToIndex(address);
        const Tag tag = AddressToTag(address);
        return lines_[index].valid_ and lines_[index].tag_ == tag;
    }

    /// @return True if the cache line corresponding to the address is valid and dirty.
    bool IsDirty(Address address) const {
        const Index index = AddressToIndex(address);
        const Tag tag = AddressToTag(address);
        return lines_[index].valid_ and lines_[index].tag_ == tag and lines_[index].dirty_;
    }

    /// @return The current cache statistics, including hits, misses, evictions, invalidations, updates, and hit rate.
    Statistics const& GetStatistics() const {
        return statistics_;
    }

    /// Saves the Cache state to a file, which can be used for debugging, testing, etc.
    bool Save(std::string const& folder) const {
        const std::filesystem::path directory{folder};
        std::filesystem::create_directory(folder);
        std::string error;
        if (not WriteBinaryFile(directory / GetFileName(), lines_.data(), lines_.size(), error)) {
            std::cerr << "Failed to save cache state: " << error << std::endl;
            return false;
        }
        return true;
    }

    /// Loads the Cache state from a file, which can be used for debugging, testing, etc.
    bool Load(std::string const& folder) {
        const std::filesystem::path directory{folder};
        std::string error;
        if (not ReadBinaryFile(directory / GetFileName(), lines_.data(), lines_.size(), error)) {
            std::cerr << "Failed to load cache state: " << error << std::endl;
            return false;
        }
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, Cache const& cache) {
        os << "Cache State:\n";
        for (size_t i = 0; i < LineCount; ++i) {
            os << "Index " << i << ": " << cache.lines_[i] << "\n";
        }
        os << "Statistics: " << cache.statistics_;
        return os;
    }

protected:
    /// @return The file name for saving or loading the cache state, based on the cache configuration.
    std::string GetFileName() const {
        return "cache_" + std::to_string(LineCount) + "x" + std::to_string(UnitsPerLine) + "x"
               + std::to_string(UnitSizeInBytes) + ".bin";
    }

    /// Helper function to reconstruct the address from the tag and index for evictions and invalidations. This is used
    /// for providing the correct address in the cache event callbacks.
    Address GetAddressFromTagAndIndex(Tag tag, Index index) const {
        uint32_t tmp = (static_cast<uint32_t>(tag) << (IndexBits + OffsetBits + UnitOffsetBits))
                       | (static_cast<uint32_t>(index) << (OffsetBits + UnitOffsetBits));
        return Address{tmp};
    }

    /// Clears the cache line at the given index, marking it as invalid and not dirty, and clearing its tag and data.
    void Clear(Index index) {
        lines_[index].valid_ = false;
        lines_[index].dirty_ = false;
        lines_[index].tag_ = Tag{0};
        std::fill(std::begin(lines_[index].data_), std::end(lines_[index].data_), UnitType{});
    }

    /// Maintains the cache line at the given index based on the provided tag, evicting or invalidating the line if it
    /// is valid and has a different tag. This is used for maintaining cache coherence in a multi-core system, for
    /// example, where we need to evict or invalidate lines that have been modified by other cores.
    void Maintenance(Index index, Tag tag) {
        if (lines_[index].valid_ and not lines_[index].dirty_ and lines_[index].tag_ != tag) {
            Invalidate(index);
        } else if (lines_[index].valid_ and lines_[index].dirty_ and lines_[index].tag_ != tag) {
            Evict(index);
        }
    }

    /// Invalidates the cache line corresponding to the given address, marking it as invalid and not dirty, and clearing
    /// its tag and data. This is used for maintaining cache coherence in a multi-core system, for example.
    void Invalidate(Index index) {
        statistics_.invalidations++;
        if (listener_) {
            listener_->OnCacheInvalidate(GetAddressFromTagAndIndex(lines_[index].tag_, index));
        }
        Clear(index);
    }

    /// Evicts the cache line at the given index, writing it back to memory if it is dirty and valid, and then
    /// invalidating the cache line.
    void Evict(Index index) {
        statistics_.evictions++;
        // In a real implementation, we would write the dirty line back to memory here before evicting it.
        if (listener_) {
            // Evicts the entire line
            listener_->OnCacheEvict(lines_[index].GetAddress(), lines_[index]);
        }
        Clear(index);
    }

    /// Helper function to handle a cache miss for the cache line at the given address, which will call the listener's
    /// OnCacheMiss callback with the address corresponding to the cache line being missed. This is used for handling
    /// cache misses that occur during partial updates, where we need to pull in the entire line from memory on a miss
    /// before doing the partial write to it, which will cause a cache miss for the line if we are doing a partial write
    /// to it, and we need to call the listener's OnCacheMiss callback with the correct address for the line being
    /// missed so that the listener can fetch the line from memory and write it to the cache before we do the partial
    /// write.
    void Miss(Address address) {
        statistics_.misses++;
        if (listener_) {
            listener_->OnCacheMiss(address);
        }
    }

    /// Helper function to handle a cache miss for the cache line at the given index, which will call the listener's
    /// OnCacheMiss callback with the address corresponding to the cache line being missed. This is used for handling
    /// cache misses that occur during partial updates, where we need to pull in the entire line from memory on a miss
    /// before doing the partial write to it, which will cause a cache miss for the line if we are doing a partial write
    /// to it, and we need to call the listener's OnCacheMiss callback with the correct address for the line being
    /// missed so that the listener can fetch the line from memory and write it to the cache before we do the partial
    /// write.
    void Miss(Index index) {
        Address line_address = GetAddressFromTagAndIndex(lines_[index].tag_, index);
        Miss(line_address);
    }

    /// Helper function to handle a cache hit for the cache line at the given address, which will call the listener's
    /// OnCacheHit callback with the address corresponding to the cache line being hit. This is used for handling cache
    /// hits that occur during partial updates, where we need to call the listener's OnCacheHit callback with the
    /// correct address for the line being hit so that the listener can perform any necessary actions on a cache hit,
    /// such as updating the LRU state of the cache line, etc.
    void Hit(Address address) {
        statistics_.hits++;
        if (listener_) {
            listener_->OnCacheHit(address);
        }
    }

    // +++ === DATA MEMBERS === +++

    Listener* listener_;  ///< A pointer to a listener for monitoring cache events

    /// The array of cache lines indexed by the cache index derived from the address which is limited to exactly the
    /// number of lines in the cache
    std::array<Line, LineCount> lines_;

    Statistics statistics_;  ///< The cache statistics for monitoring performance and behavior of the cache
};

}  // namespace isa