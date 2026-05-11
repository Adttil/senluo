#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

struct X
{
    int* copy_count;

    X(int& copy_count) : copy_count{ &copy_count }{}
    X(const X& o) : copy_count{ o.copy_count }{ ++*copy_count; }
    X(X&& o) : copy_count{ o.copy_count }{ std::puts("X(X&&);"); }
};

TEST(make, copy_time)
{
    int copy_count = 0;
    auto tpl = X{ copy_count } | refer | repeat<5> | make<tuple<X,X,X,X,X>>;
    MAGIC_CHECK(copy_count, 4);
}