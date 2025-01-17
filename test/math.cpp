#include <senluo/math.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(math, max)
{
    MAGIC_CHECK(detail::max(1), 1);
    MAGIC_CHECK(detail::max(1, 2), 2);
    MAGIC_CHECK(detail::max(2, 1), 2);
    MAGIC_CHECK(detail::max(1, 2, 3), 3);
    MAGIC_CHECK(detail::max(1, 3, 2), 3);
    MAGIC_CHECK(detail::max(2, 1, 3), 3);
    MAGIC_CHECK(detail::max(2, 3, 1), 3);
    MAGIC_CHECK(detail::max(3, 1, 2), 3);
    MAGIC_CHECK(detail::max(3, 2, 1), 3);
}

TEST(math, min)
{
    MAGIC_CHECK(detail::min(1), 1);
    MAGIC_CHECK(detail::min(1, 2), 1);
    MAGIC_CHECK(detail::min(2, 1), 1);
    MAGIC_CHECK(detail::min(1, 2, 3), 1);
    MAGIC_CHECK(detail::min(1, 3, 2), 1);
    MAGIC_CHECK(detail::min(2, 1, 3), 1);
    MAGIC_CHECK(detail::min(2, 3, 1), 1);
    MAGIC_CHECK(detail::min(3, 1, 2), 1);
    MAGIC_CHECK(detail::min(3, 2, 1), 1);
}

TEST(math, sqrt)
{
    MAGIC_CHECK(detail::sqrt(25), 5);
    MAGIC_CHECK(detail::sqrt(36), 6);
    MAGIC_CHECK(detail::sqrt(0.25), 0.5);
}