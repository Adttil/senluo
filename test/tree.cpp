#include <senluo\tree.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(subtree, aggregate)
{
    struct X { int x; float& y; double&& z; };
    float y{ 3.1f };
    double z{ 1.4 };
    X x{ 233, y, std::move(z) };

    MAGIC_CHECK(x | subtree<0>, 233);
    MAGIC_CHECK(x | subtree<3>, 233);
    MAGIC_CHECK(subtree<0>(x), 233);
    MAGIC_CHECK(x | subtree<1>, 3.1f);
    MAGIC_CHECK(x | subtree<4>, 3.1f);
    MAGIC_CHECK(x | subtree<-2>, 3.1f);
    MAGIC_CHECK(x | subtree<2>, 1.4);

    MAGIC_TCHECK(decltype(x | subtree<0>), int&);
    MAGIC_TCHECK(decltype(std::as_const(x) | subtree<0>), const int&);
    MAGIC_TCHECK(decltype(std::move(x) | subtree<0>), int&&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | subtree<0>), const int&&);

    MAGIC_TCHECK(decltype(x | subtree<1>), float&);
    MAGIC_TCHECK(decltype(std::as_const(x) | subtree<1>), float&);
    MAGIC_TCHECK(decltype(std::move(x) | subtree<1>), float&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | subtree<1>), float&);

    MAGIC_TCHECK(decltype(x | subtree<2>), double&);
    MAGIC_TCHECK(decltype(std::as_const(x) | subtree<2>), double&);
    MAGIC_TCHECK(decltype(std::move(x) | subtree<2>), double&&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(x)) | subtree<2>), double&&);
}