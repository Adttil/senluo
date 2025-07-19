#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(make, make)
{
    struct Vec2{ int x,y; };

    Vec2 v{1, 2};

    std::tuple<int, int> t = v | make<std::tuple<int, int>>;

    MAGIC_VCHECK(get<0>(t), 1);
    MAGIC_VCHECK(get<1>(t), 2);
}