#include <senluo/core.hpp>
#include <senluo/tools/constant.hpp>
#include "test_tool.hpp"

using namespace senluo;

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