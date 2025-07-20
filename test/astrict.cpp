#include <senluo/core/astrict.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(astrict, astrict)
{
    float f = 3.14f;
    double d = 6.28;
    auto t1 = tuple<int, float&, double&&>{1, f, std::move(d)};
}