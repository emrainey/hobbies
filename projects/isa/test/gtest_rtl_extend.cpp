#include <cstdint>
#include <random>

#include <gtest/gtest.h>

#include <Voperand_extend.h>

namespace {

/// The wide value as a signed 64-bit integer; the module's 66-bit result fits after sign extension.
int64_t Extend(Voperand_extend &dut, uint32_t value, int bits, bool is_signed) {
    dut.value = value;
    dut.bits = bits;
    dut.is_signed = is_signed ? 1 : 0;
    dut.eval();
    // 66 bits are held in a 3 word array; the top bits replicate the sign.
    uint64_t low = (static_cast<uint64_t>(dut.extended[1]) << 32) | dut.extended[0];
    return static_cast<int64_t>(low);
}

int64_t Reference(uint32_t value, int bits, bool is_signed) {
    const uint64_t mask = (bits == 32) ? 0xFFFFFFFFULL : ((1ULL << bits) - 1ULL);
    const uint64_t kept = value & mask;
    if (is_signed && ((kept >> (bits - 1)) & 1ULL) != 0ULL) {
        return static_cast<int64_t>(kept | ~mask);
    }
    return static_cast<int64_t>(kept);
}

}  // namespace

TEST(RtlExtend, UnsignedZeroExtends) {
    Voperand_extend dut;
    EXPECT_EQ(Extend(dut, 0xFFFFFFFFU, 8, false), 0xFF);
    EXPECT_EQ(Extend(dut, 0xFFFFFFFFU, 16, false), 0xFFFF);
    EXPECT_EQ(Extend(dut, 0xFFFFFFFFU, 32, false), 0xFFFFFFFFLL);
}

TEST(RtlExtend, SignedSignExtends) {
    Voperand_extend dut;
    EXPECT_EQ(Extend(dut, 0x80U, 8, true), -128);
    EXPECT_EQ(Extend(dut, 0x7FU, 8, true), 127);
    EXPECT_EQ(Extend(dut, 0x8000U, 16, true), -32768);
    EXPECT_EQ(Extend(dut, 0x80000000U, 32, true), -2147483648LL);
    EXPECT_EQ(Extend(dut, 0x7FFFFFFFU, 32, true), 2147483647LL);
}

TEST(RtlExtend, IgnoresBitsAboveSize) {
    Voperand_extend dut;
    EXPECT_EQ(Extend(dut, 0xABCD1234U, 8, true), 0x34);
    EXPECT_EQ(Extend(dut, 0xABCD1234U, 16, false), 0x1234);
    EXPECT_EQ(Extend(dut, 0xABCDFF80U, 8, true), -128);
}

TEST(RtlExtend, RandomMatchesReference) {
    Voperand_extend dut;
    std::mt19937 rng(0xE17E4DU);
    for (int i = 0; i < 2000; ++i) {
        const uint32_t value = rng();
        for (int bits : {8, 16, 32}) {
            for (bool is_signed : {false, true}) {
                ASSERT_EQ(Extend(dut, value, bits, is_signed), Reference(value, bits, is_signed))
                    << std::hex << value << " bits=" << std::dec << bits << " signed=" << is_signed;
            }
        }
    }
}
