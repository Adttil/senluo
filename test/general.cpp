#include <senluo/general.hpp>
#include "test_tool.hpp"

#include <senluo/macro_define.hpp>

TEST(general, fwd)
{
    struct A
    {
        int a;
        int& b;
        int&& c;
    };

    int i1{}, i2{};

    A a{ i1, i1, std::move(i2) };
    A& r = a;

    MAGIC_TCHECK(decltype(FWD(r, a)), int&);
    MAGIC_TCHECK(decltype(FWD(std::move(a), a)), int&&);
    MAGIC_TCHECK(decltype(FWD(r, b)), int&);
    MAGIC_TCHECK(decltype(FWD(std::move(a), b)), int&);
    MAGIC_TCHECK(decltype(FWD(r, c)), int&);
    MAGIC_TCHECK(decltype(FWD(std::move(a), c)), int&&);
}