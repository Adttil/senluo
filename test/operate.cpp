#include <senluo/pretreat.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(operate, transform)
{
    constexpr auto a = tuple{ 2, 0.5f } | transform([](auto x){ return x * x; });

    MAGIC_CHECK(size<decltype(a)>, 2);
    MAGIC_CHECK(a | subtree<0>, 4);
    MAGIC_CHECK(a | subtree<1>, 0.25f);
}