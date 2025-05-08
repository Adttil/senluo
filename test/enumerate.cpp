#include <senluo/enumerate.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(enumerate, general)
{
    auto i_v = tuple{ 1, 3.4 } | enumerate;
    MAGIC_CHECK(constant_t<0uz>{}, i_v | subtree<0, 0>);
    MAGIC_CHECK(1, i_v | subtree<0, 1>);
    MAGIC_CHECK(constant_t<1uz>{}, i_v | subtree<1, 0>);
    MAGIC_CHECK(3.4, i_v | subtree<1, 1>);
}