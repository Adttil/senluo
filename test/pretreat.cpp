#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(pretreatment, tuple)
{
    auto a = std::tuple{ 1,2,3 };

    auto b = a | seperate;
    MAGIC_TCHECK(decltype(b | subtree<0>), int&);
    MAGIC_TCHECK(decltype(b | subtree<1>), int&);
    MAGIC_TCHECK(decltype(b | subtree<2>), int&);
}

TEST(pretreatment, sequence)
{
    int a = 0;
    auto b = a | repeat<3> | sequence;

    MAGIC_TCHECK(decltype(b | subtree<0>), const int&);
    MAGIC_TCHECK(decltype(b | subtree<1>), const int&);
    MAGIC_TCHECK(decltype(b | subtree<2>), int&);

    MAGIC_TCHECK(decltype(3 | refer | repeat<3> | sequence | subtree<0>), const int&&);
    MAGIC_TCHECK(decltype(3 | refer | repeat<3> | sequence | subtree<1>), const int&&);
    MAGIC_TCHECK(decltype(3 | refer | repeat<3> | sequence | subtree<2>), int&&);
}

TEST(pretreatment, seperate)
{
    int a = 0;
    auto b = a | repeat<3> | seperate;

    MAGIC_TCHECK(decltype(b | subtree<0>), const int&);
    MAGIC_TCHECK(decltype(b | subtree<1>), const int&);
    MAGIC_TCHECK(decltype(b | subtree<2>), const int&);
}