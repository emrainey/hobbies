
#include <htm/htm.hpp>

int main(int argc, char *argv[]) {
    std::vector<uint16_t> s = {4, 6, 8};
    htm::dense_word<256> dense = 0b100000101000001010;
    htm::sparse_word<256> sparse = dense;
    if (sparse.test(1)) {
        sparse.set(13);
    }
    htm::sparse_word<256> s2(s);
    htm::sparse_word<256> s3 = sparse | s2;
    sparse.print();
    s3.print();
    s3.reset(13);
    s3.print();
    htm::sparse_word<256> s4{{1, 3, 17}};
    s4.print();
    htm::sparse_word<256> s5 = s3 ^ s4;
    s5.print();
    htm::sparse_word<256> s6 = s3 & s4;
    s6.print();
    htm::sparse_word<6> u1{{1, 3, 4}};
    u1.print();
    htm::sparse_word<7> u2{{3, 5}};
    u2.print();
    htm::sparse_word<13> u3 = u1 << u2;
    u3.print();
    constexpr uint32_t NW = u3.size() + u2.size();
    htm::sparse_word<NW> u4;
    constexpr uint32_t width = 100;
    for (uint32_t i = 0; i < width; i++) {
        htm::sparse_word<width> u5 = htm::scalar_encoder<width>(i, 3);
        char namepath[50];
        snprintf(namepath, sizeof(namepath), "scalar_encoding_%u_of_%u.pgm", i, width);
        u5.to_image(namepath);
    }
    {
        auto u6 = htm::dayofyear_encoder();
        auto u7 = htm::dayofmonth_encoder();
        auto u8 = htm::dayofweek_encoder();
        auto u9 = htm::dayof_encoder();
        u6.to_image("day_of_year.pgm", 7);
        u7.to_image("day_of_month.pgm", 7);
        u8.to_image("day_of_week.pgm", 7);
        u9.to_image("day_of.pgm");
    }
    return 0;
}
