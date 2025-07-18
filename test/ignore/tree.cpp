#include <senluo/core/tree.hpp>
#include <senluo/tools/constant.hpp>
#include "test_tool.hpp"
#include <magic/raw_name.h>

using namespace senluo;

TEST(tree, array)
{
    int arr[3]{ 1, 2, 3 };

    MAGIC_CHECK(tree_get<0>(arr), 1);
    MAGIC_CHECK(tree_get<1>(arr), 2);
    MAGIC_CHECK(tree_get<2>(arr), 3);

    MAGIC_TCHECK(decltype(tree_get<0>(arr)), int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::as_const(arr))), const int&);

    MAGIC_TCHECK(decltype(tree_get<0>(arr | refer)), int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::as_const(arr | refer))), const int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::move(arr) | refer)), int&&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::move(std::as_const(arr)) | refer)), const int&&);
}

TEST(tree, tuple)
{
    auto tpl = tuple{ 1, 2, 3 };

    MAGIC_CHECK(tree_get<0>(tpl), 1);
    MAGIC_CHECK(tree_get<1>(tpl), 2);
    MAGIC_CHECK(tree_get<2>(tpl), 3);

    MAGIC_TCHECK(decltype(tree_get<0>(tpl)), int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::as_const(tpl))), const int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::move(tpl))), int);
    MAGIC_TCHECK(decltype(tree_get<0>(std::move(std::as_const(tpl)))), int);

    MAGIC_TCHECK(decltype(tree_get<0>(tpl | refer)), int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::as_const(tpl | refer))), const int&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::move(tpl) | refer)), int&&);
    MAGIC_TCHECK(decltype(tree_get<0>(std::move(std::as_const(tpl)) | refer)), const int&&);
}

TEST(tree, aggregate)
{
    struct A
    {
        int x;
        float y;
        double z;
    };
    float f = 3.14f;

    static A a{ 1, f, 3.4 };

    auto&[xxx, y, c] = a;

    constexpr int* px = &tree_get<0>(a);
    constexpr auto x_name = magic::raw_name_of_member<px>();
    std::cout << x_name << '\n';

    MAGIC_CHECK(3, size<A>);
    MAGIC_CHECK(1, tree_get<0>(a));
    MAGIC_CHECK(f, tree_get<1>(a));
    MAGIC_CHECK(3.4, tree_get<2>(a));
}

TEST(tree, other)
{
    MAGIC_CHECK(size<constant_t<0>>, 0uz);
}