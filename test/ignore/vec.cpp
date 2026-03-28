#include <senluo/vec.hpp>
#include <senluo/tools/constant.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(vec, access)
{
    auto v = vec<array<int, 4>>{ 1, 2, 3, 4 };

    MAGIC_VCHECK(tree_get<0>(v), 1);
    MAGIC_VCHECK(tree_get<1>(v), 2);
    MAGIC_VCHECK(tree_get<2>(v), 3);
    MAGIC_VCHECK(tree_get<3>(v), 4);
    MAGIC_VCHECK(v.x(), 1);
    MAGIC_VCHECK(v.y(), 2);
    MAGIC_VCHECK(v.z(), 3);
    MAGIC_VCHECK(v.w(), 4);
    MAGIC_VCHECK(v.r(), 1);
    MAGIC_VCHECK(v.g(), 2);
    MAGIC_VCHECK(v.b(), 3);
    MAGIC_VCHECK(v.a(), 4);
    MAGIC_VCHECK(v.s(), 1);
    MAGIC_VCHECK(v.t(), 2);
    MAGIC_VCHECK(v.p(), 3);
    MAGIC_VCHECK(v.q(), 4);
    MAGIC_VCHECK(v.u(), 1);
    MAGIC_VCHECK(v.v(), 2);

    MAGIC_VCHECK(v.xy().x(), 1);
    MAGIC_VCHECK(v.xy().y(), 2);

    MAGIC_VCHECK(v.yx().x(), 2);
    MAGIC_VCHECK(v.yx().y(), 1);

    MAGIC_VCHECK(v.rgb().xy().u(), 1);
    
}

TEST(vec, assignment)
{
    auto v1 = vec<array<int, 4>>{ 1, 2, 3, 4 };
    constexpr auto v2 = vec<array<int, 4>>{ 5, 6, 7, 8 };
    v1 = v2.wzyx();
    MAGIC_VCHECK(v1.x(), 8);
    MAGIC_VCHECK(v1.y(), 7);
    MAGIC_VCHECK(v1.z(), 6);
    MAGIC_VCHECK(v1.w(), 5);
}

TEST(vec, arithmetic)
{
    auto v1 = vec<array<int, 4>>{ 1, 2, 3, 4 };
    auto v2 = vec<array<int, 4>>{ 5, 6, 7, 8 };

    auto v3 = v1 + v2;

    MAGIC_VCHECK(v3.x(), 6);
    MAGIC_VCHECK(v3.y(), 8);
    MAGIC_VCHECK(v3.z(), 10);
    MAGIC_VCHECK(v3.w(), 12);

    auto v4 = v1 - v2;
    MAGIC_VCHECK(v4.x(), -4);
    MAGIC_VCHECK(v4.y(), -4);
    MAGIC_VCHECK(v4.z(), -4);
    MAGIC_VCHECK(v4.w(), -4);

    auto v5 = v1 * v2;
    MAGIC_VCHECK(v5.x(), 5);
    MAGIC_VCHECK(v5.y(), 12);
    MAGIC_VCHECK(v5.z(), 21);
    MAGIC_VCHECK(v5.w(), 32);

    auto v6 = v1 / v2;
    MAGIC_VCHECK(v6.x(), 0);
    MAGIC_VCHECK(v6.y(), 0);
    MAGIC_VCHECK(v6.z(), 0);
    MAGIC_VCHECK(v6.w(), 0);

    auto v7 = v2 % v1;
    MAGIC_VCHECK(v7.x(), 0);
    MAGIC_VCHECK(v7.y(), 0);
    MAGIC_VCHECK(v7.z(), 1);
    MAGIC_VCHECK(v7.w(), 0);

    // Uncomment the following lines if you want to test unary minus operator
    auto v8 = -v1;
    MAGIC_VCHECK(v8.x(), -1);
    MAGIC_VCHECK(v8.y(), -2);
    MAGIC_VCHECK(v8.z(), -3);
    MAGIC_VCHECK(v8.w(), -4);

    auto v9 = v1.yzwx() + v2;
    MAGIC_VCHECK(v9.x(), 7);
    MAGIC_VCHECK(v9.y(), 9);
    MAGIC_VCHECK(v9.z(), 11);
    MAGIC_VCHECK(v9.w(), 9);

    auto v10 = v1.yzw() - v2.rgb();
    MAGIC_VCHECK(v10.x(), -3);
    MAGIC_VCHECK(v10.y(), -3);
    MAGIC_VCHECK(v10.z(), -3);

    auto v11 = v1 * 3;
    MAGIC_VCHECK(v11.x(), 3);
    MAGIC_VCHECK(v11.y(), 6);
    MAGIC_VCHECK(v11.z(), 9);
    MAGIC_VCHECK(v11.w(), 12);

    auto v12 = 3 * v1;
    MAGIC_VCHECK(v12.x(), 3);
    MAGIC_VCHECK(v12.y(), 6);
    MAGIC_VCHECK(v12.z(), 9);
    MAGIC_VCHECK(v12.w(), 12);

    // auto v13 = vec{array{ 2, 8 }} / 2;
    // MAGIC_CHECK(v13[0], 1);
    // MAGIC_CHECK(v13[1], 4);

    auto t1 = std::pair{ 1, 2.0 } | as_vec;
    auto t2 = std::pair{ 3, 4.0 } | as_vec;
    auto t3 = t1 + t2;
    MAGIC_TCHECK(decltype(t3), vec<std::pair<int, double>>);
}

TEST(vec, dot)
{
    constexpr auto v1 = vec<array<int, 4>>{ 1, 2, 3, 4 };
    constexpr auto v2 = vec<array<int, 4>>{ 5, 6, 7, 8 };

    constexpr auto v3 = dot(v1, v2);
    MAGIC_VCHECK(v3, 70);

    constexpr auto v4 = dot(v1.xy(), v2.uv());
    MAGIC_VCHECK(v4, 17);
}