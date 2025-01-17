#include <senluo/array.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(array, basic)
{
    auto arr = array{ 1, 2, 3 };
    
    MAGIC_TCHECK(decltype(arr), array<int, 3>);
    MAGIC_CHECK(arr, array<int, 3>{ 1, 2, 3 });

    MAGIC_CHECK(arr[0], 1);
    MAGIC_CHECK(arr[1], 2);
    MAGIC_CHECK(arr[2], 3);
    
    MAGIC_CHECK(*arr.begin(), 1);
    MAGIC_CHECK(*(arr.end() - 1), 3);
    MAGIC_CHECK(arr.size(), 3);

    MAGIC_TCHECK(decltype(arr[0]), int&);
    MAGIC_TCHECK(decltype(std::as_const(arr)[0]), const int&);
    MAGIC_TCHECK(decltype(std::move(arr)[0]), int&);
    MAGIC_TCHECK(decltype(std::move(std::as_const(arr))[0]), const int&);
}