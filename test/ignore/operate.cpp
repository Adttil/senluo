#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(operate, transform)
{
    auto tree = tuple{ 1, 3.14f } | transform([](auto&& x) { return x * 2; });

    MAGIC_CHECK(size<decltype(tree)>, 2);
    MAGIC_CHECK(tree | subtree<0>, 2);
    MAGIC_CHECK(tree | subtree<1>, 6.28f);
}