#include <senluo/core/transform.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(transform, transform)
{
    auto arr = array{ 1, 2, 3 };

    auto tree = arr | transform([](auto&& x){ return x + 1; });
    MAGIC_VCHECK(size<decltype(tree)>, 3);
    MAGIC_CHECK((tree | subtree<0>), 2, int);
    MAGIC_CHECK((tree | subtree<1>), 3, int);
    MAGIC_CHECK((tree | subtree<2>), 4, int);
}

TEST(transform, zip_transform)
{
    auto arr1 = array{ 1, 2, 3 };
    auto arr2 = array{ 4, 5, 6 };

    auto tree = zip_transform([](auto&& x, auto&& y){ return x + y; }, arr1, arr2);
    MAGIC_VCHECK(size<decltype(tree)>, 3);
    MAGIC_CHECK((tree | subtree<0>), 5, int);
    MAGIC_CHECK((tree | subtree<1>), 7, int);
    MAGIC_CHECK((tree | subtree<2>), 9, int);
}

TEST(transform, mat_transform)
{
    auto arr = array{ array{1, 2}, array{3, 4} };

    auto tree = arr | mat_transform([](auto&& x){ return x + 1; });
    MAGIC_VCHECK(size<decltype(tree)>, 2);
    MAGIC_CHECK((tree | subtree<0, 0>), 2, int);
    MAGIC_CHECK((tree | subtree<0, 1>), 3, int);
    MAGIC_CHECK((tree | subtree<1, 0>), 4, int);
    MAGIC_CHECK((tree | subtree<1, 1>), 5, int);
}

TEST(transform, mat_zip_transform)
{
    auto arr1 = array{ array{1, 2}, array{3, 4} };
    auto arr2 = array{ array{5, 6}, array{7, 8} };

    auto tree = mat_zip_transform([](auto&& x, auto&& y){ return x + y; }, arr1, arr2);
    MAGIC_VCHECK(size<decltype(tree)>, 2);
    MAGIC_CHECK((tree | subtree<0, 0>), 6, int);
    MAGIC_CHECK((tree | subtree<0, 1>), 8, int);
    MAGIC_CHECK((tree | subtree<1, 0>), 10, int);
    MAGIC_CHECK((tree | subtree<1, 1>), 12, int);
}