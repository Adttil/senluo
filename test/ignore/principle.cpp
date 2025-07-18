#include <senluo/core.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(principle, plain)
{
    using type = tuple<int, tuple<float, double>>;
    auto tpl = type{};

    MAGIC_TCHECK(decltype(tpl | principle<usage_t::once>), plain_principle<type&>);
    MAGIC_TCHECK(decltype(std::move(tpl) | principle<usage_t::once>), plain_principle<type>);
    MAGIC_TCHECK(decltype(std::move(tpl) | refer | principle<usage_t::once>), plain_principle<type&&>);

    MAGIC_TCHECK(decltype((tpl | principle<usage_t::once>).data()), type&);
    MAGIC_TCHECK(decltype((std::move(tpl) | principle<usage_t::once>).data()), type);
    MAGIC_TCHECK(decltype((std::move(tpl) | refer | principle<usage_t::once>).data()), type&&);

    //MAGIC_TCHECK(decltype((std::move(tpl) | refer | principle<usage_t::once>).data() | refer | ), type&&);

}