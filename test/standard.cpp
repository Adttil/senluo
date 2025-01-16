#include <senluo/standard.hpp>
#include "test_tool.hpp"

using namespace senluo;

struct X
{
    int a;
    std::tuple<double, float> b;
};

TEST(standard, transition)
{
    auto a = tuple{1, tuple{ 1.0, 1.5f }};

    X b = a | transition;
    MAGIC_CHECK(b | subtree<0>, 1);
    MAGIC_CHECK(1.0, b | subtree<1, 0>);
    MAGIC_CHECK(1.5f, b | subtree<1, 1>);

    X c = +wrap(a);
    MAGIC_CHECK(c | subtree<0>, 1);
    MAGIC_CHECK(1.0, c | subtree<1, 0>);
    MAGIC_CHECK(1.5f, c | subtree<1, 1>);
}

TEST(standard, plus)
{
    auto a = tuple{1, tuple{ 1.0, 1.0f }};

    auto b = X{ 2, { 0.5, 0.25f } };

    auto c = plus(a, b) | make<X>;

    MAGIC_CHECK(c | subtree<0>, 3);
    MAGIC_CHECK(1.5, c | subtree<1, 0>);
    MAGIC_CHECK(1.25f, c | subtree<1, 1>);
}