#include <senluo/tools/pipeline.hpp>
#include "test_tool.hpp"

using namespace senluo;

struct triple_fn : pipe_closure<triple_fn>
{
    constexpr auto operator()(auto x) const
    {
        return x + x + x;
    }
};

struct add_fn : pipe<add_fn>
{
    constexpr auto apply(auto x, auto y) const
    {
        return x + y;
    }
};

TEST(pipeline, pipeline)
{
    constexpr triple_fn triple{};
    constexpr add_fn add{};

    MAGIC_CHECK(add(triple(1), 3), 6);
    MAGIC_CHECK(triple(1) | add(3), 6);
    MAGIC_CHECK(add(1 | triple, 3), 6);
    MAGIC_CHECK(1 | triple | add(3), 6);
    MAGIC_CHECK(1 | (triple | add(3)), 6);
}
