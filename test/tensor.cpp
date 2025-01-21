#include <senluo/tensor.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(tensor, mat_mul)
{
    static constexpr auto a = tuple{
        tuple{ 1, 2 },
        tuple{ 3, 4 }
    };

    constexpr auto b = mat_mul(a, a) | make<array<array<int, 2>, 2>>;

    MAGIC_CHECK(7, b | subtree<0, 0>);
    MAGIC_CHECK(10, b | subtree<0, 1>);
    MAGIC_CHECK(15, b | subtree<1, 0>);
    MAGIC_CHECK(22, b | subtree<1, 1>);
}

TEST(tensor, triple_mat_mul)
{
    static constexpr auto a = tuple{
        tuple{ 1, 2 },
        tuple{ 3, 4 }
    };

    constexpr auto b = mat_mul(a, mat_mul(a, a)) | make<array<array<int, 2>, 2>>;
    constexpr auto b2 = mat_mul(a, mat_mul(a, a)) | make<cmat<2, 2, int>>;
}