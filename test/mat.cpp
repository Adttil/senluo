#include <senluo/mat.hpp>
#include "test_tool.hpp"
#include <vector>

using namespace senluo;

TEST(mat, access)
{
    auto mat = array{ array{1,3}, array{2,4} } | as_mat;

    //MAGIC_CHECK(2, ten)
    MAGIC_VCHECK(mat.r<0>().x(), 1);
    MAGIC_VCHECK(mat.r<0>().y(), 2);
    MAGIC_VCHECK(mat.r<1>().x(), 3);
    MAGIC_VCHECK(mat.r<1>().y(), 4);

    MAGIC_VCHECK(mat.c<0>().x(), 1);
    MAGIC_VCHECK(mat.c<0>().y(), 3);
    MAGIC_VCHECK(mat.c<1>().x(), 2);
    MAGIC_VCHECK(mat.c<1>().y(), 4);
}

TEST(mat, arithmetic)
{
    auto mat1 = array{ array{1,3}, array{2,4} } | as_mat;
    auto mat2 = array{ array{5,7}, array{6,8} } | as_mat;

    auto mat3 = mat1 + mat2;
    MAGIC_VCHECK(mat3.r<0>().x(), 6);
    MAGIC_VCHECK(mat3.r<0>().y(), 8);
    MAGIC_VCHECK(mat3.r<1>().x(), 10);
    MAGIC_VCHECK(mat3.r<1>().y(), 12);

    auto mat4 = mat1 - mat2;
    MAGIC_VCHECK(mat4.r<0>().x(), -4);
    MAGIC_VCHECK(mat4.r<0>().y(), -4);
    MAGIC_VCHECK(mat4.r<1>().x(), -4);
    MAGIC_VCHECK(mat4.r<1>().y(), -4);

    auto mat5 = mat1 * mat2;
    MAGIC_VCHECK(mat5.r<0>().x(), 19);
    MAGIC_VCHECK(mat5.r<0>().y(), 22);
    MAGIC_VCHECK(mat5.r<1>().x(), 43);
    MAGIC_VCHECK(mat5.r<1>().y(), 50);

    auto mat5_2 = -mat1;
    MAGIC_VCHECK(mat5_2.r<0>().x(), -1);
    MAGIC_VCHECK(mat5_2.r<0>().y(), -2);
    MAGIC_VCHECK(mat5_2.r<1>().x(), -3);
    MAGIC_VCHECK(mat5_2.r<1>().y(), -4);

    auto mat6 = mat1 * 2;
    MAGIC_VCHECK(2, mat6.rc<0, 0>());
    MAGIC_VCHECK(4, mat6.rc<0, 1>());
    MAGIC_VCHECK(6, mat6.rc<1, 0>());
    MAGIC_VCHECK(8, mat6.rc<1, 1>());

    auto mat7 = 2 * mat1;
    MAGIC_VCHECK(2, mat7.rc<0, 0>());
    MAGIC_VCHECK(4, mat7.rc<0, 1>());
    MAGIC_VCHECK(6, mat7.rc<1, 0>());
    MAGIC_VCHECK(8, mat7.rc<1, 1>());

    auto mat8 = mat2 / 2;
    MAGIC_VCHECK(2, mat8.rc<0, 0>());
    MAGIC_VCHECK(3, mat8.rc<0, 1>());
    MAGIC_VCHECK(3, mat8.rc<1, 0>());
    MAGIC_VCHECK(4, mat8.rc<1, 1>());

    auto i = mat1 * mat2;
}

TEST(mat, factory)
{
    using namespace constants;
    using mat3 = mat<array<array<float, 3>, 3>>;
    using trans_mat3 = mat<tuple<
        tuple<float, float, constant_t<0>>,
        tuple<float, float, constant_t<0>>,
        tuple<float, float, constant_t<1>>
    >>;
    using vec3 = vec<array<float, 3>>;
    
    constexpr mat3 by_rows = mat3::from_rows(
        1,2,3,
        4,5,6,
        0,0,1
    );
    MAGIC_VCHECK(1, by_rows.rc<0, 0>());
    MAGIC_VCHECK(2, by_rows.rc<0, 1>());
    MAGIC_VCHECK(3, by_rows.rc<0, 2>());
    MAGIC_VCHECK(4, by_rows.rc<1, 0>());
    MAGIC_VCHECK(5, by_rows.rc<1, 1>());
    MAGIC_VCHECK(6, by_rows.rc<1, 2>());
    MAGIC_VCHECK(0, by_rows.rc<2, 0>());
    MAGIC_VCHECK(0, by_rows.rc<2, 1>());
    MAGIC_VCHECK(1, by_rows.rc<2, 2>());

    constexpr trans_mat3 trans_by_rows = trans_mat3::from_rows(
        1,2,3,
        4,5,6,
        _0, _0, _1
    );
    MAGIC_VCHECK(1, trans_by_rows.rc<0, 0>());
    MAGIC_VCHECK(2, trans_by_rows.rc<0, 1>());
    MAGIC_VCHECK(3, trans_by_rows.rc<0, 2>());
    MAGIC_VCHECK(4, trans_by_rows.rc<1, 0>());
    MAGIC_VCHECK(5, trans_by_rows.rc<1, 1>());
    MAGIC_VCHECK(6, trans_by_rows.rc<1, 2>());

    constexpr auto by_rows2 = mat3::from_rows(
        vec3{ 1, 2, 3 },
        vec3{ 4, 5, 6 },
        vec3{ 0, 0, 1 }
    );
    MAGIC_VCHECK(1, by_rows2.rc<0, 0>());
    MAGIC_VCHECK(2, by_rows2.rc<0, 1>());
    MAGIC_VCHECK(3, by_rows2.rc<0, 2>());
    MAGIC_VCHECK(4, by_rows2.rc<1, 0>());
    MAGIC_VCHECK(5, by_rows2.rc<1, 1>());
    MAGIC_VCHECK(6, by_rows2.rc<1, 2>());
    MAGIC_VCHECK(0, by_rows2.rc<2, 0>());
    MAGIC_VCHECK(0, by_rows2.rc<2, 1>());
    MAGIC_VCHECK(1, by_rows2.rc<2, 2>());

    constexpr auto by_column = mat3::from_columns(
        1, 4, 0,
        2, 5, 0,
        3, 6, 1
    );
    MAGIC_VCHECK(1, by_column.rc<0, 0>());
    MAGIC_VCHECK(2, by_column.rc<0, 1>());
    MAGIC_VCHECK(3, by_column.rc<0, 2>());
    MAGIC_VCHECK(4, by_column.rc<1, 0>());
    MAGIC_VCHECK(5, by_column.rc<1, 1>());
    MAGIC_VCHECK(6, by_column.rc<1, 2>());
    MAGIC_VCHECK(0, by_column.rc<2, 0>());
    MAGIC_VCHECK(0, by_column.rc<2, 1>());
    MAGIC_VCHECK(1, by_column.rc<2, 2>());

    constexpr auto by_column2 = mat3::from_columns(
        vec3{ 1, 4, 0 },
        vec3{ 2, 5, 0 },
        vec3{ 3, 6, 1 }
    );
    MAGIC_VCHECK(1, by_column2.rc<0, 0>());
    MAGIC_VCHECK(2, by_column2.rc<0, 1>());
    MAGIC_VCHECK(3, by_column2.rc<0, 2>());
    MAGIC_VCHECK(4, by_column2.rc<1, 0>());
    MAGIC_VCHECK(5, by_column2.rc<1, 1>());
    MAGIC_VCHECK(6, by_column2.rc<1, 2>());
    MAGIC_VCHECK(0, by_column2.rc<2, 0>());
    MAGIC_VCHECK(0, by_column2.rc<2, 1>());
    MAGIC_VCHECK(1, by_column2.rc<2, 2>());

    constexpr auto translate2d = mat3::translate(vec{array{3.1f, 4.6f}});
    MAGIC_VCHECK(1, translate2d.rc<0, 0>());
    MAGIC_VCHECK(0, translate2d.rc<0, 1>());
    MAGIC_VCHECK(3.1f, translate2d.rc<0, 2>());
    MAGIC_VCHECK(0, translate2d.rc<1, 0>());
    MAGIC_VCHECK(1, translate2d.rc<1, 1>());
    MAGIC_VCHECK(4.6f, translate2d.rc<1, 2>());
    MAGIC_VCHECK(0, translate2d.rc<2, 0>());
    MAGIC_VCHECK(0, translate2d.rc<2, 1>());
    MAGIC_VCHECK(1, translate2d.rc<2, 2>());

    constexpr auto trans_translate2d = trans_mat3::translate(vec{array{3.1f, 4.6f}});
    MAGIC_VCHECK(1, trans_translate2d.rc<0, 0>());
    MAGIC_VCHECK(0, trans_translate2d.rc<0, 1>());
    MAGIC_VCHECK(3.1f, trans_translate2d.rc<0, 2>());
    MAGIC_VCHECK(0, trans_translate2d.rc<1, 0>());
    MAGIC_VCHECK(1, trans_translate2d.rc<1, 1>());
    MAGIC_VCHECK(4.6f, trans_translate2d.rc<1, 2>());

    constexpr auto scale2d = mat3::scale(vec{array{3.1f, 4.6f}});
    MAGIC_VCHECK(3.1f, scale2d.rc<0, 0>());
    MAGIC_VCHECK(0, scale2d.rc<0, 1>());
    MAGIC_VCHECK(0, scale2d.rc<0, 2>());
    MAGIC_VCHECK(0, scale2d.rc<1, 0>());
    MAGIC_VCHECK(4.6f, scale2d.rc<1, 1>());
    MAGIC_VCHECK(0, scale2d.rc<1, 2>());
    MAGIC_VCHECK(0, scale2d.rc<2, 0>());
    MAGIC_VCHECK(0, scale2d.rc<2, 1>());
    MAGIC_VCHECK(1, scale2d.rc<2, 2>());
    
    constexpr auto trans_scale2d = trans_mat3::scale(vec{array{3.1f, 4.6f}});
    MAGIC_VCHECK(3.1f, trans_scale2d.rc<0, 0>());
    MAGIC_VCHECK(0, trans_scale2d.rc<0, 1>());
    MAGIC_VCHECK(0, trans_scale2d.rc<0, 2>());
    MAGIC_VCHECK(0, trans_scale2d.rc<1, 0>());
    MAGIC_VCHECK(4.6f, trans_scale2d.rc<1, 1>());
    MAGIC_VCHECK(0, trans_scale2d.rc<1, 2>());

    auto rotate2d = mat3::rotate(0.5f);
    MAGIC_VCHECK(std::cos(0.5f), rotate2d.rc<0, 0>());
    MAGIC_VCHECK(-std::sin(0.5f), rotate2d.rc<0, 1>());
    MAGIC_VCHECK(0, rotate2d.rc<0, 2>());
    MAGIC_VCHECK(std::sin(0.5f), rotate2d.rc<1, 0>());
    MAGIC_VCHECK(std::cos(0.5f), rotate2d.rc<1, 1>());
    MAGIC_VCHECK(0, rotate2d.rc<1, 2>());
    MAGIC_VCHECK(0, rotate2d.rc<2, 0>());
    MAGIC_VCHECK(0, rotate2d.rc<2, 1>());
    MAGIC_VCHECK(1, rotate2d.rc<2, 2>());

    auto trans_rotate2d = trans_mat3::rotate(0.5f);
    MAGIC_VCHECK(std::cos(0.5f), trans_rotate2d.rc<0, 0>());
    MAGIC_VCHECK(-std::sin(0.5f), trans_rotate2d.rc<0, 1>());
    MAGIC_VCHECK(0, trans_rotate2d.rc<0, 2>());
    MAGIC_VCHECK(std::sin(0.5f), trans_rotate2d.rc<1, 0>());
    MAGIC_VCHECK(std::cos(0.5f), trans_rotate2d.rc<1, 1>());
    MAGIC_VCHECK(0, trans_rotate2d.rc<1, 2>());

    auto t1 = mat<array<array<float, 1>, 1>>::from_rows(1);
    auto t2 = mat<array<array<float, 1>, 1>>::from_rows(vec{array{1}});

}