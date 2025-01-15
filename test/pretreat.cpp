#include <senluo/pretreat.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(pretreatment, sequence)
{
    int a = 0;
    auto b = a | repeat<3> | sequence;

    MAGIC_TCHECK(decltype(std::move(b) | subtree<0>), const int&);
    MAGIC_TCHECK(decltype(std::move(b) | subtree<1>), const int&);
    MAGIC_TCHECK(decltype(std::move(b) | subtree<2>), int&);
}

TEST(pretreatment, seperate)
{
    int a = 0;
    auto b = a | repeat<3> | seperate;

    MAGIC_TCHECK(decltype(std::move(b) | subtree<0>), const int&);
    MAGIC_TCHECK(decltype(std::move(b) | subtree<1>), const int&);
    MAGIC_TCHECK(decltype(std::move(b) | subtree<2>), const int&);
}