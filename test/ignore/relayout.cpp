#include <senluo/core/relayout.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(relayout, basic)
{
    auto arr = array{ 1, 2, 3 };
    auto tree = arr | relayout<tuple{ 2, 1, 0 }>;

    MAGIC_VCHECK((size<decltype(tree)>), 3);
    MAGIC_VCHECK(tree | subtree<0>, 3);
    MAGIC_VCHECK(tree | subtree<1>, 2);
    MAGIC_VCHECK(tree | subtree<2>, 1);
    MAGIC_VCHECK(tree | subtree<3>, 3);
}

TEST(relayout, component)
{
    auto tree = array{ array{1, 2}, array{3, 4} } | component<1, 1>;

    MAGIC_VCHECK(size<decltype(tree)>, 2);
    MAGIC_VCHECK(tree | subtree<0>, 2);
    MAGIC_VCHECK(tree | subtree<1>, 4);
}