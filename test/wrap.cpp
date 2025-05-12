#include <senluo/core.hpp>
#include <senluo/tools/constant.hpp>
#include "test_tool.hpp"

using namespace senluo;

template<class T>
void foo(T&& t)
{
    [&]<size_t...I>(std::index_sequence<I...>){
        if constexpr(std::is_rvalue_reference_v<T>)
        {}
        else{
        static_assert(std::same_as<decltype(t), T&&>);
        }
    }(std::make_index_sequence<sizeof(T)>{});
}

TEST(wrap, unwrap)
{
    int i = 0;
    int& ref = i;
    foo(ref);

    wrapper<int> o{ i };
    wrapper<int&> l{ i };
    wrapper<int&&> r{ std::move(i) };

    using type = tuple<relayout_tree<std::plus<>, tuple<array<size_t, 0>, array<size_t, 0>, array<size_t, 0>, array<size_t, 0>>{}>, const std::array<int, 4>&, const std::array<int, 4>&>;

    constexpr auto UsageTree = senluo::tuple_ns::tuple<senluo::tuple_ns::tuple<senluo::usage_t, senluo::usage_t, senluo::usage_t, senluo::usage_t>, senluo::tuple_ns::tuple<senluo::usage_t, senluo::usage_t, senluo::usage_t, senluo::usage_t>, senluo::tuple_ns::tuple<senluo::usage_t, senluo::usage_t, senluo::usage_t, senluo::usage_t> >{senluo::tuple_ns::tuple<senluo::usage_t, senluo::usage_t, senluo::usage_t, senluo::usage_t>{senluo::usage_t::once, senluo::usage_t::none, senluo::usage_t::none, senluo::usage_t::none}, senluo::tuple_ns::tuple<senluo::usage_t, senluo::usage_t, senluo::usage_t, senluo::usage_t>{senluo::usage_t::once, senluo::usage_t::none, senluo::usage_t::none, senluo::usage_t::none}, senluo::tuple_ns::tuple<senluo::usage_t, senluo::usage_t, senluo::usage_t, senluo::usage_t>{senluo::usage_t::once, senluo::usage_t::none, senluo::usage_t::none, senluo::usage_t::none}};

    using ttt = principle_t<type&, UsageTree>;

    MAGIC_TCHECK(unwrap_t<type&>, type&);
    MAGIC_TCHECK(unwrap_fwd_t<type&>, type&);

    MAGIC_TCHECK(decltype(unwrap(i)), int&);
    MAGIC_TCHECK(decltype(unwrap(std::move(i))), int);
    MAGIC_TCHECK(decltype(unwrap(o)), int&);
    MAGIC_TCHECK(decltype(unwrap(std::move(o))), int);
    MAGIC_TCHECK(decltype(unwrap(l)), int&);
    MAGIC_TCHECK(decltype(unwrap(std::move(l))), int&);
    MAGIC_TCHECK(decltype(unwrap(r)), int&);
    MAGIC_TCHECK(decltype(unwrap(std::move(r))), int&&);
}

TEST(wrap, unwrap_fwd)
{
    int i = 0;
    //int& ref = std::move(i);
    //int&& lref = ref;
    wrapper<int> o{ i };
    wrapper<int&> l{ i };
    wrapper<int&&> r{ std::move(i) };

    MAGIC_TCHECK(decltype(unwrap_fwd(i)), int&);
    MAGIC_TCHECK(decltype(unwrap_fwd(std::move(i))), int&&);
    MAGIC_TCHECK(decltype(unwrap_fwd(o)), int&);
    MAGIC_TCHECK(decltype(unwrap_fwd(std::move(o))), int&&);
    MAGIC_TCHECK(decltype(unwrap_fwd(l)), int&);
    MAGIC_TCHECK(decltype(unwrap_fwd(std::move(l))), int&);
    MAGIC_TCHECK(decltype(unwrap_fwd(r)), int&);
    MAGIC_TCHECK(decltype(unwrap_fwd(std::move(r))), int&&);
}

TEST(wrap, unwrap_base)
{
    int i = 0;

    MAGIC_TCHECK(decltype((std::move(i) | repeat<3>).unwrap_base()), int&&);
    MAGIC_TCHECK(decltype((std::move(i) | refer | repeat<3>).unwrap_base()), wrapper<int&&>);
    MAGIC_TCHECK(decltype((std::move(i) | repeat<3> | refer).unwrap_base()), wrapper<int&&>);
    MAGIC_TCHECK(decltype((std::move(i) | refer | repeat<3> | refer).unwrap_base()), wrapper<int&&>);

    MAGIC_TCHECK(decltype((i | repeat<3>).unwrap_base()), int&);
    MAGIC_TCHECK(decltype((i | refer | repeat<3>).unwrap_base()), int&);
    MAGIC_TCHECK(decltype((i | repeat<3> | refer).unwrap_base()), int&);
    MAGIC_TCHECK(decltype((i | refer | repeat<3> | refer).unwrap_base()), int&);
}