#include <senluo/adaptor.hpp>
#include "test_tool.hpp"

using namespace senluo;

struct Square : adaptor_closure<Square>
{
    auto operator()(auto x) const
    {
        return x * x;
    }
};
inline  constexpr Square square{};

TEST(adaptor, adaptor_closure)
{
    MAGIC_CHECK(square(2), 4);
    MAGIC_CHECK(2 | square, 4);
    MAGIC_CHECK(2 | square | square, 16);
    MAGIC_CHECK(2 | (square | square), 16);
}

struct Add : adaptor<Add>
{
    auto adapt(auto x, auto y) const
    {
        return x + y;
    }
};
inline constexpr Add add{};

TEST(adaptor, adaptor)
{
    MAGIC_CHECK(add(2, 3), 5);
    MAGIC_CHECK(2 | add(3), 5);
    MAGIC_CHECK(2 | add(3) | add(4), 9);
    MAGIC_CHECK(2 | (add(3) | add(4)), 9);
    MAGIC_CHECK(2 | add(3) | square, 25);
    MAGIC_CHECK(2 | (add(3) | square), 25);
}