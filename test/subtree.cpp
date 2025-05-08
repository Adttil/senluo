#include "senluo/core/subtree.hpp"
#include "test_tool.hpp"

using namespace senluo;

TEST(subtree, array)
{
    auto arr = array{ 1, 2, 3 };
    
    MAGIC_CHECK(subtree<0>(arr), 1);
    MAGIC_CHECK(subtree<1>(arr), 2);
    MAGIC_CHECK(subtree<2>(arr), 3);

    MAGIC_TCHECK(decltype(subtree<0>(arr)), int&);
    MAGIC_TCHECK(decltype(subtree<0>(std::as_const(arr))), const int&);
    MAGIC_TCHECK(decltype(subtree<0>(std::move(arr))), int);
    MAGIC_TCHECK(decltype(subtree<0>(std::move(std::as_const(arr)))), int);

    MAGIC_TCHECK(decltype(subtree<0>(arr | refer)), int&);
    MAGIC_TCHECK(decltype(subtree<0>(std::as_const(arr | refer))), const int&);
    MAGIC_TCHECK(decltype(subtree<0>(std::move(arr) | refer)), int&&);
    MAGIC_TCHECK(decltype(subtree<0>(std::move(std::as_const(arr)) | refer)), const int&&);
}

TEST(subtree, multi_index)
{
    auto tpl = tuple{ array{ 1, 2 }, tuple{ 3.14f, array{ 3, 4 } } };

    MAGIC_CHECK(1, subtree<0, 0>(tpl));
    MAGIC_CHECK(2, subtree<0, 1>(tpl));
    MAGIC_CHECK(3.14f, subtree<1, 0>(tpl));
    MAGIC_CHECK(3, subtree<1, 1, 0>(tpl));
    MAGIC_CHECK(4, subtree<1, 1, 1>(tpl));

    MAGIC_CHECK(1, subtree<array{0, 0}>(tpl));
    MAGIC_CHECK(2, subtree<array{0, 1}>(tpl));
    MAGIC_CHECK(3.14f, subtree<array{1, 0}>(tpl));
    MAGIC_CHECK(3, subtree<array{1, 1, 0}>(tpl));
    MAGIC_CHECK(4, subtree<array{1, 1, 1}>(tpl));

    MAGIC_CHECK(3, subtree<array{1, 1}, 0>(tpl));
    MAGIC_CHECK(3, subtree<1, array{1, 0}>(tpl));
    MAGIC_CHECK(3, subtree<1, array{1}, array{0}>(tpl));
}