#include "basal/gtest_helper.hpp"

#include <htm/htm.hpp>

TEST(DenseWords, Constructors) {
    htm::dense_word<256> dense = 0b100000101000001010;
    ASSERT_EQ(256u, dense.size());
    ASSERT_TRUE(dense.any());
    ASSERT_EQ(5u, dense.count());
}

TEST(SparseWords, Constructors) {
    htm::sparse_word<256> sparse;
    ASSERT_EQ(256u, sparse.size());
    ASSERT_FALSE(sparse.any());
    ASSERT_TRUE(sparse.none());
}

TEST(SparseWords, CopyConstructors) {
    {
        htm::dense_word<256> dense = 0b100000101000001010;
        htm::sparse_word<256> sparse{dense};
        std::cout << "sparse: " << sparse << std::endl;
        ASSERT_EQ(5u, sparse.count());
        ASSERT_TRUE(sparse.any());
        ASSERT_TRUE(sparse.test(1));
    }
    {
        std::vector<uint16_t> vec = {4, 6, 8};
        htm::sparse_word<256> sparse{vec};
        std::cout << "sparse: " << sparse << std::endl;
        ASSERT_EQ(3u, sparse.count());
        ASSERT_TRUE(sparse.any());
        ASSERT_TRUE(sparse.test(4));
    }
}


TEST(SparseWords, CopyAssign) {
    {
        htm::dense_word<256> dense = 0b100000101000001010;
        htm::sparse_word<256> sparse;
        sparse = dense;
        std::cout << "sparse: " << sparse << std::endl;
        ASSERT_EQ(5u, sparse.count());
        ASSERT_TRUE(sparse.any());
        ASSERT_TRUE(sparse.test(1));
    }
}

TEST(SparseWords, API) {
    htm::sparse_word<256> sparse;

    sparse.set(13);
    ASSERT_TRUE(sparse.test(13));
    sparse.reset(13);
    ASSERT_FALSE(sparse.test(13));

    for (size_t i = 0; i < 256; i++) {
        sparse.set(i);
    }
    ASSERT_TRUE(sparse.all());
    sparse.clear();
    ASSERT_TRUE(sparse.none());
}

TEST(SparseWords, Comparison) {
    htm::sparse_word<256> a;
    htm::sparse_word<256> b;

    a.set(13);
    b.set(13);
    ASSERT_EQ(a, b);
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);

    b.set(14);
    ASSERT_NE(a, b);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
}

TEST(SparseWords, BitOperations) {
    htm::sparse_word<256> a;
    htm::sparse_word<256> b;

    a.set(13);
    b.set(14);

    auto c = a & b;
    ASSERT_FALSE(c.test(13));
    ASSERT_FALSE(c.test(14));

    c = a | b;
    ASSERT_TRUE(c.test(13));
    ASSERT_TRUE(c.test(14));

    c = a ^ b;
    ASSERT_TRUE(c.test(13));
    ASSERT_TRUE(c.test(14));
}

TEST(SparseWords, Conjoin) {
    htm::sparse_word<13> a;
    htm::sparse_word<17> b;

    a.set(9);
    b.set(11);

    auto c = a + b;
    ASSERT_EQ(30u, c.size());
    ASSERT_EQ(2u, c.count());
    ASSERT_TRUE(c.test(0+9)); // lower bit
    ASSERT_TRUE(c.test(13+11));  //upper bit
}
