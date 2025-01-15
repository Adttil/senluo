#include <senluo/astrict.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(astrict, array)
{
    constexpr auto astrict_table = tuple{ stricture_t::readonly, stricture_t::none };

    auto a = array{ 1, 2 };

    auto astrict_a = a | astrict<astrict_table>;

    MAGIC_CHECK(astrict_a | subtree<0>, 1);
    MAGIC_CHECK(astrict_a | subtree<1>, 2);

    MAGIC_TCHECK(decltype(astrict_a | subtree<0>), const int&);
    MAGIC_TCHECK(decltype(astrict_a | subtree<1>), int&);
}