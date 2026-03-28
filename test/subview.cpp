#include <senluo/core/subview.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(tree, array)
{
    int a[] = { 1, 2, 3 };

    MAGIC_VCHECK(child_count_of(a), 3uz);
    MAGIC_VCHECK(child_count<decltype(a)>, 3uz);

    MAGIC_CHECK((subview<0>(a)), 1, int&);
    MAGIC_CHECK((subview<0>(std::as_const(a))), 1, const int&);
    MAGIC_CHECK((subview<0>(std::move(a))), 1, int&&);
    MAGIC_CHECK((subview<0>(std::move(std::as_const(a)))), 1, const int&&);

    MAGIC_CHECK((subview<1>(a)), 2, int&);

    MAGIC_CHECK((subview<2>(a)), 3, int&);
}

TEST(tree, tuple)
{
    float f = 2.0f;
    double d = 3.0;
    auto t = tuple<int, float&, double&&>{ 1, f, std::move(d) };
    
    MAGIC_VCHECK(child_count<decltype(t)>, 3uz);

    MAGIC_CHECK((subview<0>(t)), 1, int&);
    MAGIC_CHECK((subview<0>(std::as_const(t))), 1, const int&);
    MAGIC_CHECK((subview<0>(std::move(t))), 1, int&&);
    MAGIC_CHECK((subview<0>(std::move(std::as_const(t)))), 1, const int&&);

    MAGIC_CHECK((subview<1>(t)), 2.0f, float&);
    MAGIC_CHECK((subview<1>(std::as_const(t))), 2.0f, float&);
    MAGIC_CHECK((subview<1>(std::move(t))), 2.0f, float&);
    MAGIC_CHECK((subview<1>(std::move(std::as_const(t)))), 2.0f, float&);

    MAGIC_CHECK((subview<2>(t)), 3.0, double&);
    MAGIC_CHECK((subview<2>(std::as_const(t))), 3.0, double&);
    MAGIC_CHECK((subview<2>(std::move(t))), 3.0, double&&);
    MAGIC_CHECK((subview<2>(std::move(std::as_const(t)))), 3.0, double&&);
    
}

TEST(tree, aggregate)
{
    struct S
    {
        int i;
        float& f;
        double&& d;
    };

    float f = 2.0f;
    double d = 3.0;

    S s{ 1, f, std::move(d) };

    MAGIC_VCHECK(child_count<S>, 3uz);

    MAGIC_CHECK((subview<0>(s)), 1, int&);
    MAGIC_CHECK((subview<0>(std::as_const(s))), 1, const int&);
    MAGIC_CHECK((subview<0>(std::move(s))), 1, int&&);
    MAGIC_CHECK((subview<0>(std::move(std::as_const(s)))), 1, const int&&);

    MAGIC_CHECK((subview<1>(s)), 2.0f, float&);
    MAGIC_CHECK((subview<1>(std::as_const(s))), 2.0f, float&);
    MAGIC_CHECK((subview<1>(std::move(s))), 2.0f, float&);
    MAGIC_CHECK((subview<1>(std::move(std::as_const(s)))), 2.0f, float&);

    MAGIC_CHECK((subview<2>(s)), 3.0, double&);
    MAGIC_CHECK((subview<2>(std::as_const(s))), 3.0, double&);
    MAGIC_CHECK((subview<2>(std::move(s))), 3.0, double&&);
    MAGIC_CHECK((subview<2>(std::move(std::as_const(s)))), 3.0, double&&);
}

template<class T>
struct X
{
    T base;

    template<size_t I, class Self>
    constexpr decltype(auto) subview(this Self&& self, custom_t = {}) noexcept
    {

        return std::forward_like<Self>(self.base);
    }

    static consteval size_t get_child_count(custom_t = {})
    {
        return 3uz;
    }
};

TEST(tree, custom)
{
    int i = 0;

    X<int> x{ 0 };

    MAGIC_VCHECK(child_count<X<int>>, 3uz);
    MAGIC_CHECK((subview<0>(x)), 0, int&);
    MAGIC_CHECK((subview<0>(std::as_const(x))), 0, const int&);
    MAGIC_CHECK((subview<0>(std::move(x))), 0, int&&);
    MAGIC_CHECK((subview<0>(std::move(std::as_const(x)))), 0, const int&&);
}