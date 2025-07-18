#include <senluo/core/tree.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(tree, array)
{
    int a[] = { 1, 2, 3 };

    MAGIC_VCHECK(size<decltype(a)>, 3uz);

    MAGIC_CHECK((tree_get<0>(a)), 1, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(a))), 1, const int&);
    MAGIC_CHECK((tree_get<0>(std::move(a))), 1, int);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(a)))), 1, int);
    MAGIC_CHECK((tree_get<0>(refer(std::move(a)))), 1, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(a))))), 1, const int&&);

    MAGIC_CHECK((tree_get<1>(a)), 2, int&);

    MAGIC_CHECK((tree_get<2>(a)), 3, int&);
}

TEST(tree, tuple)
{
    float f = 2.0f;
    double d = 3.0;
    auto t = tuple<int, float&, double&&>{ 1, f, std::move(d) };
    
    MAGIC_VCHECK(size<decltype(t)>, 3uz);

    MAGIC_CHECK((tree_get<0>(t)), 1, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(t))), 1, const int&);
    MAGIC_CHECK((tree_get<0>(std::move(t))), 1, int);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(t)))), 1, int);
    MAGIC_CHECK((tree_get<0>(refer(std::move(t)))), 1, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(t))))), 1, const int&&);

    MAGIC_CHECK((tree_get<1>(t)), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(std::as_const(t))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(std::move(t))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(std::move(std::as_const(t)))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(refer(std::move(t)))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(refer(std::move(std::as_const(t))))), 2.0f, float&);

    MAGIC_CHECK((tree_get<2>(t)), 3.0, double&);
    MAGIC_CHECK((tree_get<2>(std::as_const(t))), 3.0, double&);
    MAGIC_CHECK((tree_get<2>(std::move(t))), 3.0, double&&);
    MAGIC_CHECK((tree_get<2>(std::move(std::as_const(t)))), 3.0, double&&);
    MAGIC_CHECK((tree_get<2>(refer(std::move(t)))), 3.0, double&&);
    MAGIC_CHECK((tree_get<2>(refer(std::move(std::as_const(t))))), 3.0, double&&);
    
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

    MAGIC_VCHECK(size<S>, 3uz);

    MAGIC_CHECK((tree_get<0>(s)), 1, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(s))), 1, const int&);
    MAGIC_CHECK((tree_get<0>(std::move(s))), 1, int);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(s)))), 1, int);
    MAGIC_CHECK((tree_get<0>(refer(std::move(s)))), 1, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(s))))), 1, const int&&);

    MAGIC_CHECK((tree_get<1>(s)), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(std::as_const(s))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(std::move(s))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(std::move(std::as_const(s)))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(refer(std::move(s)))), 2.0f, float&);
    MAGIC_CHECK((tree_get<1>(refer(std::move(std::as_const(s))))), 2.0f, float&);

    MAGIC_CHECK((tree_get<2>(s)), 3.0, double&);
    MAGIC_CHECK((tree_get<2>(std::as_const(s))), 3.0, double&);
    MAGIC_CHECK((tree_get<2>(std::move(s))), 3.0, double&&);
    MAGIC_CHECK((tree_get<2>(std::move(std::as_const(s)))), 3.0, double&&);
    MAGIC_CHECK((tree_get<2>(refer(std::move(s)))), 3.0, double&&);
    MAGIC_CHECK((tree_get<2>(refer(std::move(std::as_const(s))))), 3.0, double&&);
}

template<class T>
struct X
{
    wrapper<T> base;

    template<size_t I, class Self>
    constexpr decltype(auto) tree_get(this Self&& self, custom_t = {}) noexcept
    {
        return unwrap_opt_refer<Self>(self.base);
    }
};

template<class T>
struct senluo::tree_size<X<T>> : std::integral_constant<size_t, 3uz> {};

template<class T>
struct Y
{
    wrapper<T> base;

    template<size_t I, derived_from<Y> Self>
    friend constexpr decltype(auto) tree_get(Self&& self, custom_t) noexcept
    {
        return unwrap_opt_refer<Self>(self.base);
    }
};

template<class T>
struct senluo::tree_size<Y<T>> : std::integral_constant<size_t, 3uz> {};

TEST(tree, custom)
{
    int i = 0;

    X<int> x{ 0 };

    MAGIC_VCHECK(size<X<int>>, 3uz);
    MAGIC_CHECK((tree_get<0>(x)), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(x))), 0, const int&);
    MAGIC_CHECK((tree_get<0>(std::move(x))), 0, int);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(x)))), 0, int);
    MAGIC_CHECK((tree_get<0>(refer(std::move(x)))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(x))))), 0, const int&&);

    X<int&> xr{ i };

    MAGIC_VCHECK(size<X<int&>>, 3uz);
    MAGIC_CHECK((tree_get<0>(xr)), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(xr))), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::move(xr))), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(xr)))), 0, int&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(xr)))), 0, int&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(xr))))), 0, int&);

    X<int&&> xrr{ std::move(i) };

    MAGIC_VCHECK(size<X<int&&>>, 3uz);
    MAGIC_CHECK((tree_get<0>(xrr)), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(xrr))), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::move(xrr))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(xrr)))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(xrr)))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(xrr))))), 0, int&&);

    Y<int> y{ 0 };

    MAGIC_VCHECK(size<Y<int>>, 3uz);
    MAGIC_CHECK((tree_get<0>(y)), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(y))), 0, const int&);
    MAGIC_CHECK((tree_get<0>(std::move(y))), 0, int);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(y)))), 0, int);
    MAGIC_CHECK((tree_get<0>(refer(std::move(y)))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(y))))), 0, const int&&);

    Y<int&> yr{ i };

    MAGIC_VCHECK(size<Y<int&>>, 3uz);
    MAGIC_CHECK((tree_get<0>(yr)), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(yr))), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::move(yr))), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(yr)))), 0, int&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(yr)))), 0, int&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(yr))))), 0, int&);

    Y<int&&> yrr{ std::move(i) };

    MAGIC_VCHECK(size<Y<int&&>>, 3uz);
    MAGIC_CHECK((tree_get<0>(yrr)), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::as_const(yrr))), 0, int&);
    MAGIC_CHECK((tree_get<0>(std::move(yrr))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(std::move(std::as_const(yrr)))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(yrr)))), 0, int&&);
    MAGIC_CHECK((tree_get<0>(refer(std::move(std::as_const(yrr))))), 0, int&&);
}