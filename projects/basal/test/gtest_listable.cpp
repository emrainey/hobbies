#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <cstdint>

using namespace basal;

class Dummy {
public:
    Dummy() : value{0u} {}
    Dummy(uint32_t x) : value{x} {
    }
    bool operator==(const Dummy& v) const {
        return value == v.value;
    }
    bool operator==(uint32_t b) const {
        return value == b;
    }

protected:
    uint32_t value;
};

TEST(Listable, Single) {
    listable<Dummy> d{Dummy{7}};
    ASSERT_EQ(d, d);
}

TEST(Listable, Move) {
    listable<Dummy> d{Dummy{7}};
    listable<Dummy> e{std::move(d)};
    ASSERT_TRUE(e.as_base() == uint32_t{7u});
    ASSERT_EQ(nullptr, e.prev());
    ASSERT_EQ(nullptr, e.next());

    listable<Dummy> f = std::move(e);
    ASSERT_TRUE(f.as_base() == uint32_t{7u});
    ASSERT_EQ(nullptr, f.prev());
    ASSERT_EQ(nullptr, f.next());
}

TEST(Listable, List) {
    listable<Dummy> d{Dummy{7}};
    listable<Dummy> e{Dummy{8}};
    listable<Dummy> f{Dummy{9}};
    e.insert_prev(&d);
    e.insert_next(&f);

    ASSERT_EQ(&d, e.prev());
    ASSERT_EQ(&f, e.next());
}

TEST(Listable, ListedCtorMove) {
    listable<Dummy> d{Dummy{7}};
    listable<Dummy> e{Dummy{8}};
    listable<Dummy> f{Dummy{9}};
    e.insert_prev(&d);
    e.insert_next(&f);
    listable<Dummy> g{std::move(e)};

    ASSERT_EQ(&d, g.prev());
    ASSERT_EQ(&f, g.next());
}

TEST(Listable, ListedAssignMove) {
    listable<Dummy> d{Dummy{7}};
    listable<Dummy> e{Dummy{8}};
    listable<Dummy> f{Dummy{9}};
    e.insert_prev(&d);
    e.insert_next(&f);
    listable<Dummy> g = std::move(e);

    ASSERT_EQ(&d, g.prev());
    ASSERT_EQ(&f, g.next());
}

TEST(Listable, Iteration) {
    listable<Dummy> d{Dummy{7}};
    listable<Dummy> e{Dummy{8}};
    listable<Dummy> f{Dummy{9}};
    e.insert_prev(&d);
    e.insert_next(&f);

    const listable<Dummy>* tmp = d.as_listable();
    ASSERT_NE(nullptr, tmp);
    ASSERT_EQ((void*)&d, (void*)tmp);
    ASSERT_EQ((void*)&e, (void*)tmp->next());
    ASSERT_EQ((void*)&f, (void*)tmp->next()->next());
    ASSERT_EQ(nullptr, (void*)tmp->next()->next()->next());

    tmp = f.as_listable();
    ASSERT_NE(nullptr, tmp);
    ASSERT_EQ((void*)&f, (void*)tmp);
    ASSERT_EQ((void*)&e, (void*)tmp->prev());
    ASSERT_EQ((void*)&d, (void*)tmp->prev()->prev());
    ASSERT_EQ(nullptr, (void*)tmp->prev()->prev()->prev());

    ASSERT_EQ((void*)&f, (void*)tmp->prev()->next());

}