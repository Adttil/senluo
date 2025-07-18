#include <senluo/vec.hpp>
#include <senluo/tools/constant.hpp>
#include "test_tool.hpp"

using namespace senluo;

// TEST(vec, access)
// {
//     auto v = vec<array<int, 4>>{ 1, 2, 3, 4 };

//     MAGIC_CHECK(v[0], 1);
//     MAGIC_CHECK(v[1], 2);
//     MAGIC_CHECK(v[2], 3);
//     MAGIC_CHECK(v[3], 4);
//     MAGIC_CHECK(v.x(), 1);
//     MAGIC_CHECK(v.y(), 2);
//     MAGIC_CHECK(v.z(), 3);
//     MAGIC_CHECK(v.w(), 4);
//     MAGIC_CHECK(v.r(), 1);
//     MAGIC_CHECK(v.g(), 2);
//     MAGIC_CHECK(v.b(), 3);
//     MAGIC_CHECK(v.a(), 4);
//     MAGIC_CHECK(v.s(), 1);
//     MAGIC_CHECK(v.t(), 2);
//     MAGIC_CHECK(v.p(), 3);
//     MAGIC_CHECK(v.q(), 4);
//     MAGIC_CHECK(v.u(), 1);
//     MAGIC_CHECK(v.v(), 2);

//     MAGIC_CHECK(v.xy().x(), 1);
//     MAGIC_CHECK(v.xy().y(), 2);

//     MAGIC_CHECK(v.yx().x(), 2);
//     MAGIC_CHECK(v.yx().y(), 1);

//     MAGIC_CHECK(v.rgb().xy().u(), 1);
    
// }

// TEST(vec, assignment)
// {
//     auto v1 = vec<array<int, 4>>{ 1, 2, 3, 4 };
//     constexpr auto v2 = vec<array<int, 4>>{ 5, 6, 7, 8 };
//     v1 = v2.wzyx();
//     MAGIC_CHECK(v1.x(), 8);
//     MAGIC_CHECK(v1.y(), 7);
//     MAGIC_CHECK(v1.z(), 6);
//     MAGIC_CHECK(v1.w(), 5);
// }

TEST(vec, arithmetic)
{
    auto v1 = vec<array<int, 4>>{ 1, 2, 3, 4 };
    auto v2 = vec<array<int, 4>>{ 5, 6, 7, 8 };

    auto v3 = v1 + v2;

    // MAGIC_CHECK(v3[0], 6);
    // MAGIC_CHECK(v3[1], 8);
    // MAGIC_CHECK(v3[2], 10);
    // MAGIC_CHECK(v3[3], 12);

    // auto v4 = v1 - v2;
    // MAGIC_CHECK(v4[0], -4);
    // MAGIC_CHECK(v4[1], -4);
    // MAGIC_CHECK(v4[2], -4);
    // MAGIC_CHECK(v4[3], -4);

    // auto v5 = v1 * v2;
    // MAGIC_CHECK(v5[0], 5);
    // MAGIC_CHECK(v5[1], 12);
    // MAGIC_CHECK(v5[2], 21);
    // MAGIC_CHECK(v5[3], 32);

    // auto v6 = v1 / v2;
    // MAGIC_CHECK(v6[0], 0);
    // MAGIC_CHECK(v6[1], 0);
    // MAGIC_CHECK(v6[2], 0);
    // MAGIC_CHECK(v6[3], 0);

    // auto v7 = v1 % v2;
    // MAGIC_CHECK(v7[0], 1);
    // MAGIC_CHECK(v7[1], 2);
    // MAGIC_CHECK(v7[2], 3);
    // MAGIC_CHECK(v7[3], 4);

    // auto v7_2 = -v1;
    // MAGIC_CHECK(v7_2[0], -1);
    // MAGIC_CHECK(v7_2[1], -2);
    // MAGIC_CHECK(v7_2[2], -3);
    // MAGIC_CHECK(v7_2[3], -4);

    // auto v8 = v1.yzwx() + v2;
    // auto v9 = v1.abgr() + v2;
    // MAGIC_CHECK(v8[0], 7);

    // auto v10 = v1.yzw() - v2.rgb();
    // MAGIC_CHECK(v10[0], -3);
    // MAGIC_CHECK(v10[1], -3);
    // MAGIC_CHECK(v10[2], -3);

    // auto v11 = v1 * 3;
    // MAGIC_CHECK(v11[0], 3);
    // MAGIC_CHECK(v11[1], 6);
    // MAGIC_CHECK(v11[2], 9);
    // MAGIC_CHECK(v11[3], 12);

    // auto v12 = 3 * v1;
    // MAGIC_CHECK(v12[0], 3);
    // MAGIC_CHECK(v12[1], 6);
    // MAGIC_CHECK(v12[2], 9);
    // MAGIC_CHECK(v12[3], 12);

    // auto v13 = vec{array{ 2, 8 }} / 2;
    // MAGIC_CHECK(v13[0], 1);
    // MAGIC_CHECK(v13[1], 4);

    // auto t1 = std::pair{ 1, 2.0 } | as_vec;
    // auto t2 = std::pair{ 3, 4.0 } | as_vec;
    // auto t3 = t1 + t2;
    // MAGIC_TCHECK(decltype(t3), vec<std::pair<int, double>>);
}

// namespace ns {
//     struct A{};

//     template<class T>
//     constexpr auto plus(const vec<A>& a, const vec<T>& b)
//     {
//         return 0;
//     }
// }

// TEST(vec, dot)
// {
//     constexpr auto v1 = vec<array<int, 4>>{ 1, 2, 3, 4 };
//     constexpr auto v2 = vec<array<int, 4>>{ 5, 6, 7, 8 };

//     constexpr auto v3 = dot(v1, v2);
//     MAGIC_CHECK(v3, 70);

//     constexpr auto v4 = dot(v1.xy(), v2.uv());
//     MAGIC_CHECK(v4, 17);

//     auto vt = vec<ns::A>{};
//     auto vt2 = vt + vt;

//     MAGIC_TCHECK(decltype(vt2), int);
//     MAGIC_CHECK(vt2, 0);
// }