#include <senluo/relayout.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(relayout, array)
{
    constexpr auto layout = tuple{ array{ 1uz }, array{ 0uz } };

    auto a = array{ 1, 2 };

    auto relayout_a = a | relayout<layout>;

    //MAGIC_CHECK((size<decltype(relayout_a)>), 2);
    MAGIC_CHECK(relayout_a | subtree<0>, 2);
    MAGIC_CHECK(relayout_a | subtree<1>, 1);

    MAGIC_TCHECK(decltype(std::move(a) | refer | relayout<layout>), relayout_tree<array<int, 2>&&, layout>);
    MAGIC_TCHECK(decltype(std::move(a) | relayout<layout>), relayout_tree<array<int, 2>, layout>);
}

TEST(relayout, repeat)
{
    auto a = array{ 1, 2 };
    auto b = a | repeat<2>;
    MAGIC_CHECK(1, b | subtree<0, 0>);
    MAGIC_CHECK(2, b | subtree<0, 1>);
    MAGIC_CHECK(1, b | subtree<1, 0>);
    MAGIC_CHECK(2, b | subtree<1, 1>);
}

TEST(relayout, transpose)
{
    auto a = array{ array{ 1, 2 }, array{ 3, 4 } };
    auto b = a | transpose<>;
    MAGIC_CHECK(1, b | subtree<0, 0>);
    MAGIC_CHECK(3, b | subtree<0, 1>);
    MAGIC_CHECK(2, b | subtree<1, 0>);
    MAGIC_CHECK(4, b | subtree<1, 1>);
}