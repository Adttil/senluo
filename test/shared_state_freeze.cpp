#include <senluo/core/shared_state_freeze.hpp>

#include <utility>

#include "test_tool.hpp"

using namespace senluo;

template<class T>
struct wrapper
{
    T value;
};

template<class T>
constexpr wrapper<const T&> shared_state_freeze(wrapper<T&> t)
{
    return { t.value };
}

namespace senluo
{
    template<class T>
    inline constexpr auto mutable_state_sharing_mode_of<wrapper<T>> = mutable_state_sharing_mode_of_aggregate<T>;
}

TEST(shared_state_freeze, mutable_state_sharing_mode_defaults)
{
    using enum mutable_state_sharing_mode;
    MAGIC_CHECK(mutable_state_sharing_mode_of<int>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<int*>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const int*>, exclusive);

    MAGIC_CHECK(mutable_state_sharing_mode_of<double>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<double*>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const double*>, exclusive);

    enum enum_t{
        first,
        second
    };

    MAGIC_CHECK(mutable_state_sharing_mode_of<enum_t>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<enum_t*>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const enum_t*>, exclusive);

    MAGIC_CHECK(mutable_state_sharing_mode_of<nullptr_t>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<nullptr_t*>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const nullptr_t*>, exclusive);

    MAGIC_CHECK(mutable_state_sharing_mode_of<enum_t[3]>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<enum_t(*)[3]>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const enum_t(*)[3]>, exclusive);

    MAGIC_CHECK(mutable_state_sharing_mode_of<enum_t[3][3]>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<enum_t(*)[3][3]>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const enum_t(*)[3][3]>, exclusive);

    MAGIC_CHECK(mutable_state_sharing_mode_of<nullptr_t[3]>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<nullptr_t(*)[3]>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const nullptr_t(*)[3]>, exclusive);

    MAGIC_CHECK(mutable_state_sharing_mode_of<nullptr_t[3][3]>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<nullptr_t(*)[3][3]>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<const nullptr_t(*)[3][3]>, exclusive);
}

TEST(shared_state_freeze, mutable_state_sharing_mode_specialization)
{
    using enum mutable_state_sharing_mode;

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int*>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int* const>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int* const&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<int* const&&>>, shared);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int*>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int* const>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int* const&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const int* const&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double*>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double* const>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double* const&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<double* const&&>>, shared);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double*>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double* const>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double* const&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const double* const&&>>, no_mutable_state);

    enum enum_t {
        first,
        second
    };

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t*>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t* const>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t* const&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t* const&&>>, shared);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t*>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t* const>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t* const&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t* const&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t&&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t*>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t* const>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t* const&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t* const&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t*>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t* const>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t*&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t* const&>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t*&&>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t* const&&>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t[3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t(&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t(&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t(&&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t(&&)[3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (*)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (* const)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (*&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (* const&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (*&&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (* const&&)[3]>>, shared);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (*)[3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (* const)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (*&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (* const&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (*&&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (* const&&)[3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t[3][3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t(&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t(&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t(&&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t(&&)[3][3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (*)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (* const)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (*&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (* const&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (*&&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<enum_t (* const&&)[3][3]>>, shared);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (*)[3][3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (* const)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (*&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (* const&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (*&&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const enum_t (* const&&)[3][3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t(&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t(&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t(&&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t(&&)[3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (*)[3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (* const)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (*&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (* const&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (*&&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (* const&&)[3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (*)[3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (* const)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (*&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (* const&)[3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (*&&)[3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (* const&&)[3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t(&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t(&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t(&&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t(&&)[3][3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (*)[3][3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (* const)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (*&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (* const&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (*&&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<nullptr_t (* const&&)[3][3]>>, no_mutable_state);

    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (*)[3][3]>>, exclusive);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (* const)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (*&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (* const&)[3][3]>>, no_mutable_state);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (*&&)[3][3]>>, shared);
    MAGIC_CHECK(mutable_state_sharing_mode_of<wrapper<const nullptr_t (* const&&)[3][3]>>, no_mutable_state);
}

TEST(shared_state_freeze, mutable_state_sharing_mode_of_aggregate)
{
    struct unknown_t { int* p; };
    using enum mutable_state_sharing_mode;

    // empty aggregate
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<>), no_mutable_state);

    // std::nullptr_t: no mutable state
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t&>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const std::nullptr_t&>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t&&>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const std::nullptr_t&&>), no_mutable_state);

    // int: exclusive mutable state
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int&&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&&>), no_mutable_state);

    // unknown_t: default shared mutable state
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t&&>), shared);

    // no_mutable_state combined with no_mutable_state
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, const std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t&, const std::nullptr_t&>), no_mutable_state);

    // exclusive combined with no_mutable_state
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, std::nullptr_t>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, const std::nullptr_t>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, const std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&, std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&&, std::nullptr_t>), no_mutable_state);

    // exclusive combined with exclusive
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, const int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, const int>), no_mutable_state);

    // reference-to-exclusive becomes shared when non-const
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int&&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, int&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, int&&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, int&>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&, int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&&, int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&, const int>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&&, const int>), no_mutable_state);

    // shared dominates no_mutable_state
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t&, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&&, std::nullptr_t>), shared);

    // shared dominates exclusive
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t, int>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t, int>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t&, int>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&, int>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&&, int>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t, const int>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, unknown_t>), shared);

    // shared combined with shared
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t, unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t, const unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const unknown_t, const unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t&, const unknown_t&>), shared);

    // three-member combinations
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, std::nullptr_t, std::nullptr_t>), no_mutable_state);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, int, std::nullptr_t>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, int, const int>), exclusive);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, const int, const int>), no_mutable_state);

    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<std::nullptr_t, int, unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<unknown_t, int, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int, unknown_t, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int, const std::nullptr_t, unknown_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<int&, const int&, std::nullptr_t>), shared);
    MAGIC_CHECK((mutable_state_sharing_mode_of_aggregate<const int&, const int&&, std::nullptr_t>), no_mutable_state);
}

TEST(shared_state_freeze, alias_frozen_default)
{
    MAGIC_CHECK(shared_state_frozen<int>, true);
    MAGIC_CHECK(shared_state_frozen<const int>, true);
    MAGIC_CHECK(shared_state_frozen<int&>, false);
    MAGIC_CHECK(shared_state_frozen<const int&>, true);
    MAGIC_CHECK(shared_state_frozen<int&&>, false);
    MAGIC_CHECK(shared_state_frozen<const int&&>, true);

    MAGIC_CHECK(shared_state_frozen<double>, true);
    MAGIC_CHECK(shared_state_frozen<const double>, true);
    MAGIC_CHECK(shared_state_frozen<double&>, false);
    MAGIC_CHECK(shared_state_frozen<const double&>, true);
    MAGIC_CHECK(shared_state_frozen<double&&>, false);
    MAGIC_CHECK(shared_state_frozen<const double&&>, true);
    
    enum enum_t{
        first,
        second
    };

    MAGIC_CHECK(shared_state_frozen<enum_t>, true);
    MAGIC_CHECK(shared_state_frozen<const enum_t>, true);
    MAGIC_CHECK(shared_state_frozen<enum_t&>, false);
    MAGIC_CHECK(shared_state_frozen<const enum_t&>, true);
    MAGIC_CHECK(shared_state_frozen<enum_t&&>, false);
    MAGIC_CHECK(shared_state_frozen<const enum_t&&>, true);

    MAGIC_CHECK(shared_state_frozen<nullptr_t>, true);
    MAGIC_CHECK(shared_state_frozen<const nullptr_t>, true);
    MAGIC_CHECK(shared_state_frozen<nullptr_t&>, true);
    MAGIC_CHECK(shared_state_frozen<const nullptr_t&>, true);
    MAGIC_CHECK(shared_state_frozen<nullptr_t&&>, true);
    MAGIC_CHECK(shared_state_frozen<const nullptr_t&&>, true);
}

TEST(shared_state_freeze, alias_frozen_specializations)
{
    MAGIC_CHECK(shared_state_frozen<wrapper<int>>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int>>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<int&>>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int&>>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<int&&>>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int&&>>, true);

    MAGIC_CHECK(shared_state_frozen<const wrapper<int>>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int>>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<int&>>, false);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int&>>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<int&&>>, false);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int&&>>, true);

    MAGIC_CHECK(shared_state_frozen<wrapper<int>&>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int>&>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<int&>&>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int&>&>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<int&&>&>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int&&>&>, true);

    MAGIC_CHECK(shared_state_frozen<const wrapper<int>&>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int>&>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<int&>&>, false);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int&>&>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<int&&>&>, false);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int&&>&>, true);

    MAGIC_CHECK(shared_state_frozen<wrapper<int>&&>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int>&&>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<int&>&&>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int&>&&>, true);
    MAGIC_CHECK(shared_state_frozen<wrapper<int&&>&&>, false);
    MAGIC_CHECK(shared_state_frozen<wrapper<const int&&>&&>, true);

    MAGIC_CHECK(shared_state_frozen<const wrapper<int>&&>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int>&&>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<int&>&&>, false);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int&>&&>, true);
    MAGIC_CHECK(shared_state_frozen<const wrapper<int&&>&&>, false);
    MAGIC_CHECK(shared_state_frozen<const wrapper<const int&&>&&>, true);
}

TEST(shared_state_freeze, no_mutable_state)
{
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<nullptr_t>(nullptr)), nullptr_t);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const nullptr_t>(nullptr)), nullptr_t);

    nullptr_t nullptr_value;

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<nullptr_t&>(nullptr_value)), nullptr_t&);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const nullptr_t&>(nullptr_value)), const nullptr_t&);

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<nullptr_t&&>(nullptr)), nullptr_t&&);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const nullptr_t&&>(nullptr)), const nullptr_t&&);
}

TEST(shared_state_freeze, exclusive)
{
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<int>(3)), int);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const int>(3)), int);

    int value = 3;

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<int&>(value)), const int&);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const int&>(value)), const int&);

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<int&&>(3)), const int&&);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const int&&>(3)), const int&&);
}

TEST(shared_state_freeze, default_shared)
{
    struct unknown_type{ int* p; };

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<unknown_type>({})), unknown_shared_state_frozen_type_of<unknown_type>);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const unknown_type>({})), unknown_shared_state_frozen_type_of<const unknown_type>);

    unknown_type value;

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<unknown_type&>(value)), unknown_shared_state_frozen_type_of<unknown_type&>);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const unknown_type&>(value)), unknown_shared_state_frozen_type_of<const unknown_type&>);

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<unknown_type&&>({})), unknown_shared_state_frozen_type_of<unknown_type&&>);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const unknown_type&&>({})), unknown_shared_state_frozen_type_of<const unknown_type&&>);
}

TEST(shared_state_freeze, specialization)
{
    int value = 3;
    wrapper<int&> ref_box{ value };

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<wrapper<int&>>(std::move(ref_box))), wrapper<const int&>);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const wrapper<int&>>(std::move(ref_box))), wrapper<const int&>);

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<wrapper<int&>&>(ref_box)), wrapper<const int&>);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const wrapper<int&>&>(ref_box)), wrapper<const int&>);

    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<wrapper<int&>&&>(std::move(ref_box))), wrapper<const int&>);
    MAGIC_TCHECK(decltype(senluo::shared_state_freeze<const wrapper<int&>&&>(std::move(ref_box))), wrapper<const int&>);
}