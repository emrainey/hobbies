#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <isa/cache.hpp>

using namespace isa;

using TestCache = Cache<uint32_t, 4, 4>;

// Lets test some assumptions about how this works
static_assert(TestCache::LineSizeInBytes == 16, "Line size in bytes should be 16 for 4 units of 4 bytes each");

// These two addresses _should_ map to the same index but different tags in the cache, which will allow us to test
// evictions and updates properly.
constexpr Address TestCacheAddress1{0x1000};
constexpr Address TestCacheAddress2{0x2000};
static_assert(TestCache::AddressToIndex(TestCacheAddress1) == TestCache::AddressToIndex(TestCacheAddress2),
              "These addresses should map to the same index in the cache");
static_assert(TestCache::AddressToTag(TestCacheAddress1) != TestCache::AddressToTag(TestCacheAddress2),
              "These addresses should map to different tags in the cache");

// Incrementing the address for up to but not including 16 bytes should also map to the same line.
static_assert(TestCache::AddressToIndex(TestCacheAddress1) == TestCache::AddressToIndex(TestCacheAddress1 + 15U),
              "These addresses should map to the same index in the cache");

constexpr static uint32_t TestData1[TestCache::UnitsPerLine] = {0xDEADBEEF, 0xBAADF00D, 0xFEEDFACE, 0xDEADC0DE};

class MockListener : public TestCache::Listener {
public:
    MOCK_METHOD(void, OnCacheHit, (Address), (override));
    MOCK_METHOD(void, OnCacheMiss, (Address), (override));
    MOCK_METHOD(void, OnCacheEvict, (Address, TestCache::Line const&), (override));
    MOCK_METHOD(void, OnCacheInvalidate, (Address), (override));
    MOCK_METHOD(void, OnCacheUpdate, (Address), (override));
};

class CacheTest : public testing::Test {
protected:
    MockListener listener;
    TestCache cache{&listener};
    uint32_t data[TestCache::UnitsPerLine] = {0};
    const Address address{TestCacheAddress1};
    const Address colliding_address{TestCacheAddress2};

    CacheTest() {
        std::copy(std::begin(TestData1), std::end(TestData1), data);
    }

    void ExpectFetchMissAndPopulate(Address miss_address) {
        EXPECT_CALL(listener, OnCacheMiss(miss_address))
            .WillOnce(testing::Invoke([&, miss_address](Address addr) -> void {
                EXPECT_EQ(addr, miss_address);
                cache.Update(miss_address, data);
            }));
    }

    void FetchMissUpdateHit(Address test_address, TestCache::UnitType* test_data) {
        TestCache::Line line{};
        EXPECT_CALL(listener, OnCacheMiss(test_address))
            .WillOnce(testing::Invoke([&, test_address](Address addr) -> void {
                EXPECT_EQ(addr, test_address);
                EXPECT_TRUE(cache.Update(test_address, test_data));
            }));
        EXPECT_CALL(listener, OnCacheUpdate(test_address)).Times(1);
        EXPECT_FALSE(cache.Fetch(test_address, line));
        EXPECT_TRUE(cache.HasHit(test_address));
        EXPECT_FALSE(cache.IsDirty(test_address));
        testing::Mock::VerifyAndClearExpectations(&listener);
        // ===================================================================
        // clear out the data so we can be sure it is coming from the cache and not just left over from the previous
        // write
        memset(&line.data_, 0, sizeof(line.data_));
        EXPECT_CALL(listener, OnCacheHit(test_address)).Times(1);
        EXPECT_TRUE(cache.Fetch(test_address, line));
        testing::Mock::VerifyAndClearExpectations(&listener);
        // ===================================================================
        std::cout << "Cache after update: " << cache << std::endl;
        for (size_t i = 0; i < TestCache::UnitsPerLine; ++i) {
            EXPECT_EQ(line.data_[i], TestData1[i]);
        }
    }

    void ExpectStats(uint32_t hits, uint32_t misses, uint32_t updates, uint32_t evictions, uint32_t invalidations) {
        auto stats = cache.GetStatistics();
        EXPECT_EQ(stats.hits, hits);
        EXPECT_EQ(stats.misses, misses);
        EXPECT_EQ(stats.updates, updates);
        EXPECT_EQ(stats.evictions, evictions);
        EXPECT_EQ(stats.invalidations, invalidations);
    }
};

TEST_F(CacheTest, Creation) {
    for (Address address = 0; address < TestCache::TotalSize; address += TestCache::LineSizeInBytes) {
        EXPECT_FALSE(cache.HasHit(address));
        EXPECT_FALSE(cache.IsDirty(address));
    }
}

TEST_F(CacheTest, FetchMissUpdateHit) {
    FetchMissUpdateHit(address, data);
    ExpectStats(1, 1, 1, 0, 0);
}

TEST_F(CacheTest, FetchMissUpdateHitEvict) {
    FetchMissUpdateHit(address, data);
    ExpectStats(1, 1, 1, 0, 0);

    // now write again to the same address to trigger an eviction of the dirty line but with a different address which
    // will map to a different tag but the same index, which should cause an invalidation of the previous line
    Address new_address{colliding_address};  // This address should map to the same index but a different tag, which
                                             // will cause an invalidation of the previous line
    EXPECT_CALL(listener, OnCacheEvict(address, testing::_)).Times(0);
    EXPECT_CALL(listener, OnCacheUpdate(new_address)).Times(1);
    // The original address should no longer be a hit after the eviction.
    EXPECT_CALL(listener, OnCacheHit(address)).Times(0);
    EXPECT_TRUE(cache.Update(new_address, data));
    EXPECT_TRUE(cache.HasHit(new_address));
    // The original address should have been invalidated, so it should no longer be a hit.
    EXPECT_FALSE(cache.HasHit(address));
    // The original address should have been invalidated, so it should no longer be dirty.
    EXPECT_FALSE(cache.IsDirty(address));
    // The new address should not be dirty after the write since it is now clean with the new data.
    EXPECT_FALSE(cache.IsDirty(new_address));

    std::cout << "Cache after 2nd write: " << cache << std::endl;

    // check statistics again after the invalidation
    ExpectStats(1, 1, 2, 0, 1);
}

TEST_F(CacheTest, Invalidate) {
    FetchMissUpdateHit(address, data);
    ExpectStats(1, 1, 1, 0, 0);

    TestCache::Line line{};
    EXPECT_TRUE(cache.Fetch(address, line));  // should hit and fetch the line

    Address old_address{TestCacheAddress1};
    Address new_address{colliding_address};
    EXPECT_CALL(listener, OnCacheUpdate(new_address)).Times(1);
    EXPECT_CALL(listener, OnCacheInvalidate(old_address)).Times(1);
    EXPECT_CALL(listener, OnCacheInvalidate(new_address)).Times(0);
    EXPECT_TRUE(cache.Update(new_address,
                             data));  // this should an invalidation since it is not dirty, but is does collide by index
    EXPECT_FALSE(cache.HasHit(old_address));
    EXPECT_TRUE(cache.HasHit(new_address));
    EXPECT_FALSE(cache.IsDirty(old_address));
    EXPECT_FALSE(cache.IsDirty(new_address));

    EXPECT_CALL(listener, OnCacheInvalidate(new_address)).Times(1);
    // Manually invalidate the cache line, like an instruction could
    cache.Invalidate(address);
    EXPECT_FALSE(cache.HasHit(address));
    EXPECT_FALSE(cache.IsDirty(address));

    ExpectStats(2, 1, 2, 0, 2);
}

TEST_F(CacheTest, FetchMissUpdateHitPartialUpdateEvict) {
    FetchMissUpdateHit(address, data);
    ExpectStats(1, 1, 1, 0, 0);

    // Now perform a partial write to the same cache line with a different address that maps to the same index but a
    // different tag, which should cause an eviction of the dirty line.
    Address new_address{colliding_address};  // This address should map to the same index but a different tag, which
                                             // will cause an eviction of the dirty line when we pull in the new line.
    uint32_t new_data[TestCache::UnitsPerLine];
    std::copy(std::begin(TestData1), std::end(TestData1),
              new_data);  // New data to write to the cache line on the partial write.
    EXPECT_CALL(listener, OnCacheEvict(address, testing::_)).Times(0);
    EXPECT_CALL(listener, OnCacheHit(address))
        .Times(0);  // The original address should no longer be a hit after the eviction.

    // Perform a partial write to the new address with an offset that will write to the second unit in the cache line's
    // data array.
    const uint32_t offset = 1;  // This will write to the second unit in the cache line's data array.
    const Address partial_update_address{new_address() + (offset * sizeof(uint32_t))};
    EXPECT_CALL(listener, OnCacheUpdate(partial_update_address)).Times(1);
    new_data[offset] = 0xBAADFACE;  // This is the new data that we will write to the second unit in the cache line's
                                    // data array on the partial write.
    EXPECT_TRUE(cache.Update(partial_update_address, &new_data[offset], sizeof(uint32_t)));
    // === Call should be been done by now ^
    EXPECT_TRUE(cache.HasHit(new_address));
    // The original address should have been evicted, so it should no longer be a hit.
    EXPECT_FALSE(cache.HasHit(address));
    // The original address should have been evicted, so it should no longer be dirty.
    EXPECT_FALSE(cache.IsDirty(address));
    EXPECT_TRUE(cache.IsDirty(new_address));  // The new address should be dirty after the write.

    std::cout << "Cache after partial write: " << cache << std::endl;

    ExpectStats(1, 1, 2, 0, 0);
}