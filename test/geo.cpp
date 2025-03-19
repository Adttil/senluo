#include <senluo/geo.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(geo, plus)
{
    geo::vec<3> a{ 1.0f, 2.0f, 3.0f };
    MAGIC_CHECK(size<geo::vec<3>>, 3);
    wrap(a) + wrap(a);
    zip_transform(std::plus<>{}, a, a);
    auto b = a + a;
    MAGIC_CHECK(b.x(), 2.0f);
    MAGIC_CHECK(b.y(), 4.0f);
    MAGIC_CHECK(b.z(), 6.0f);
    //(a | wrap) + (a | wrap); 
}