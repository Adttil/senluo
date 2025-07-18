#include <senluo/core/tag.hpp>
#include "test_tool.hpp"

using namespace senluo;
using namespace senluo::detail;

TEST(tag, used_order_and_count)
{
    constexpr auto usage = tuple{ usage_t::once, usage_t::once, usage_t::once };
    constexpr auto layout = tuple{ indexes_of_whole, indexes_of_whole, indexes_of_whole };
    constexpr auto base_stricture = stricture_t::none;
    using base_shape_t = tuple<>;

    constexpr auto order_and_count = used_order_and_count<layout, usage, base_stricture, base_shape_t>;

    MAGIC_CHECK(order_and_count.used_count_tree, 3);
    MAGIC_CHECK(order_and_count.used_order_tree, tuple{ 0uz, 1uz, 2uz });

    constexpr auto seq_strc = sequence_stricture_v<layout, usage, base_stricture, base_shape_t>;
    constexpr auto inv_seq_strc = inverse_sequence_stricture_v<layout, usage, base_stricture, base_shape_t>;
    constexpr auto sep_strc = seperate_stricture_v<layout, usage, base_stricture, base_shape_t>;

    MAGIC_CHECK(seq_strc, tuple{ stricture_t::readonly, stricture_t::readonly, stricture_t::none });
    MAGIC_CHECK(inv_seq_strc, tuple{ stricture_t::none, stricture_t::readonly, stricture_t::readonly });
    MAGIC_CHECK(sep_strc, stricture_t::readonly);
}