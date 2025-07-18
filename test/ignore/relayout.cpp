#include "senluo/core/relayout.hpp"
#include "test_tool.hpp"

using namespace senluo;

TEST(relayout, basic)
{
    auto arr = array{ 1, 2, 3 };
    auto tree = arr | relayout<tuple{ 2, 1, 0 }>;

    MAGIC_CHECK(size<decltype(tree)>, 3);
    MAGIC_CHECK(tree | subtree<0>, 3);
    MAGIC_CHECK(tree | subtree<1>, 2);
    MAGIC_CHECK(tree | subtree<2>, 1);
    MAGIC_CHECK(tree | subtree<3>, 3);
}

TEST(relayout, repeat)
{
    auto tree = 1 | repeat<3>;

    MAGIC_CHECK(tree | subtree<0>, 1);
    MAGIC_CHECK(tree | subtree<1>, 1);
    MAGIC_CHECK(tree | subtree<2>, 1);

    auto tree2 = 1 | repeat<3> | repeat<3>;

    MAGIC_CHECK(1, tree2 | subtree<0, 0>);
    MAGIC_CHECK(1, tree2 | subtree<0, 1>);
    MAGIC_CHECK(1, tree2 | subtree<0, 2>);
}

TEST(relayout, repeat_by_shape)
{
    auto tree = 1 | repeat_as<int[2][2]>;

    MAGIC_CHECK(1, tree | subtree<0, 0>);
    MAGIC_CHECK(1, tree | subtree<0, 1>);
    MAGIC_CHECK(1, tree | subtree<1, 0>);
    MAGIC_CHECK(1, tree | subtree<1, 1>);
}   

TEST(relayout, component)
{
    auto tree = array{ array{1, 2}, array{3, 4} } | component<1, 1>;

    MAGIC_CHECK(size<decltype(tree)>, 2);
    MAGIC_CHECK(tree | subtree<0>, 2);
    MAGIC_CHECK(tree | subtree<1>, 4);
}