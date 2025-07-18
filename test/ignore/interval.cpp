#include <senluo/interval.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(interval, access)
{
    auto r = std::pair{ 1, 3 } | as_interval;

    MAGIC_CHECK(r.min(), 1);
    MAGIC_CHECK(r.max(), 3);
}

TEST(interval, contains)
{
    auto r = std::pair{ 1, 3 } | as_interval;

    MAGIC_CHECK(r.contains(0), false);
    MAGIC_CHECK(r.contains(1), true);
    MAGIC_CHECK(r.contains(2), true);
    MAGIC_CHECK(r.contains(3), false);
    MAGIC_CHECK(r.contains(4), false);
}