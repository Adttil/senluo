#include <senluo/adaptor.hpp>
#include "test_tool.hpp"

#include <senluo/macro_define.hpp>

struct AddOne : senluo::adaptor_closure<AddOne>
{
    auto operator()(auto x)const
    {
        return x + 1;
    }
};

struct Square : senluo::adaptor_closure<Square>
{
    auto operator()(auto x)const
    {
        return x * x;
    }
};

TEST(adaptor_closure, _)
{
    //using namespace senluo;
    static constexpr AddOne add_one{};
    static constexpr Square square{};
    constexpr auto mix = add_one | square;

    MAGIC_CHECK(1 | add_one, 2);
    MAGIC_CHECK(1 | add_one | square, 4);
    MAGIC_CHECK(1 | (add_one | square), 4);
    MAGIC_CHECK(1 | mix, 4);
}