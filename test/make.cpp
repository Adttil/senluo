#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(make, tree_basic)
{

    constexpr auto x = 233 | make<int>;
    constexpr auto a = array{ 1, 3 } | make<std::tuple<int, int>>;
    
    constexpr auto tpl = tuple{ 1, 3.14f } | make<std::tuple<int, float>>;

    MAGIC_CHECK(tpl | subtree<0>, 1);
    MAGIC_CHECK(tpl | subtree<1>, 3.14f);
}

struct X
{
private:
    size_t* copy_count;
public:
    X(size_t& copy_count) : copy_count(&copy_count) {}
    X(const X& x) : copy_count(x.copy_count){ ++*copy_count; };
    X(X&& x) : copy_count(x.copy_count) { std::puts("move"); };
};

TEST(make, auto_move)
{
    size_t copy_count = 0;
    X{ copy_count } | refer | repeat<5> | make<array<X, 5>>;
    MAGIC_CHECK(copy_count, 4);
}

TEST(make, aggregate)
{
    struct Foo
    {
        X x;
        X y;
        X z;
    };
    MAGIC_CHECK(aggregate_tree<Foo>, true);
    
    size_t copy_count = 0;
    X{ copy_count } | refer | repeat<3> | make<Foo>;
    MAGIC_CHECK(copy_count, 2);
}

TEST(make, transform)
{
    constexpr auto a = tuple{ 2, 0.5f } | transform([](auto x){ return x * x; }) | make<std::tuple<int, float>>;

    MAGIC_CHECK(a | subtree<0>, 4);
    MAGIC_CHECK(a | subtree<1>, 0.25f);
}

struct fn
{
    constexpr auto operator()(auto x)const
    {
        return x * x;
    }
};

TEST(make, transform_transform)
{
    constexpr auto a = tuple{ 2, 0.5f } 
                        | transform(fn{})
                        | transform(fn{})
                        | make<std::tuple<int, float>>;

    MAGIC_CHECK(a | subtree<0>, 16);
    MAGIC_CHECK(a | subtree<1>, 0.0625f);
}