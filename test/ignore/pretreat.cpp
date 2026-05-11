#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(pretreat, plain)
{
    using type = tuple<int, tuple<float, double>>;
    auto tpl = type{};

    MAGIC_TCHECK(decltype(tpl | sequence), type&);
    MAGIC_TCHECK(decltype(tpl | refer | sequence), type&);
    MAGIC_TCHECK(decltype(std::move(tpl) | sequence), type);
    MAGIC_TCHECK(decltype(std::move(tpl) | refer | sequence), wrapper<type&&>);
}