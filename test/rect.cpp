#include <senluo/rect.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(rect, access)
{
    auto r = tuple{ std::pair{ 1, 3 }, std::pair{ 2, 4 } } | as_rect;

    MAGIC_CHECK(r.min().x(), 1);
    MAGIC_CHECK(r.min().y(), 2);
    MAGIC_CHECK(r.max().x(), 3);
    MAGIC_CHECK(r.max().y(), 4);
}

TEST(rect, contains)
{
    auto r = tuple{ std::pair{ 1, 3 }, std::pair{ 2, 4 } } | as_rect;

    MAGIC_CHECK(r.contains(std::pair{ 0, 0 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 1, 0 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 2, 0 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 3, 0 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 4, 0 }), false);

    MAGIC_CHECK(r.contains(std::pair{ 1, 0 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 1, 1 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 1, 2 }), true);
    MAGIC_CHECK(r.contains(std::pair{ 1, 3 }), true);
    MAGIC_CHECK(r.contains(std::pair{ 1, 4 }), false);
    MAGIC_CHECK(r.contains(std::pair{ 1, 5 }), false);
}