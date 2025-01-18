#include <senluo\tree.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(tree, indexical)
{
    MAGIC_CHECK(true, indexical<int>);
    MAGIC_CHECK(true, indexical<char>);
    MAGIC_CHECK(true, indexical<size_t>);
    MAGIC_CHECK(true, indexical<array<int, 0>>);
    MAGIC_CHECK(true, indexical<array<unsigned char, 3>>);
    MAGIC_CHECK(true, indexical<std::array<int, 0>>);
    MAGIC_CHECK(true, indexical<std::array<unsigned char, 3>>);
}

TEST(tree, size)
{
    MAGIC_CHECK(0, size<int>);
    MAGIC_CHECK(0, size<tuple<>>);
    MAGIC_CHECK(0, size<std::tuple<>>);
    MAGIC_CHECK(0, size<array<int, 0>>);
    MAGIC_CHECK(0, size<decltype([]{})>);
    
    MAGIC_CHECK(3, size<int[3]>);
    MAGIC_CHECK(2, size<tuple<int, float>>);
    MAGIC_CHECK(2, size<std::tuple<int, float>>);
    MAGIC_CHECK(4, size<array<int, 4>>);

    struct X{ int x; int& y; int&& z; float& w; std::tuple<int, float> u; };
    MAGIC_CHECK(5, size<X>);
}

TEST(tree, subtree)
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