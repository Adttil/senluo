#include <senluo/core/external_freeze.hpp>

#include <utility>

#include "test_tool.hpp"

using namespace senluo;

template<class T>
struct wrapper
{
    T value;
};

template<class T>
constexpr wrapper<const T&> external_freeze(wrapper<T&> t)
{
    return { t.value };
}

namespace senluo
{
    template<class T>
    inline constexpr auto writability_through<wrapper<T>> = writability_through_aggregate<T>;
}

TEST(external_freeze, mutable_state_sharing_mode_defaults)
{
    using enum writability;
    MAGIC_CHECK(writability_through<int>, self);
    MAGIC_CHECK(writability_through<int*>, external);
    MAGIC_CHECK(writability_through<const int*>, self);

    MAGIC_CHECK(writability_through<double>, self);
    MAGIC_CHECK(writability_through<double*>, external);
    MAGIC_CHECK(writability_through<const double*>, self);

    enum enum_t{
        first,
        second
    };

    MAGIC_CHECK(writability_through<enum_t>, self);
    MAGIC_CHECK(writability_through<enum_t*>, external);
    MAGIC_CHECK(writability_through<const enum_t*>, self);

    MAGIC_CHECK(writability_through<nullptr_t>, none);
    MAGIC_CHECK(writability_through<nullptr_t*>, self);
    MAGIC_CHECK(writability_through<const nullptr_t*>, self);

    MAGIC_CHECK(writability_through<enum_t[3]>, self);
    MAGIC_CHECK(writability_through<enum_t(*)[3]>, external);
    MAGIC_CHECK(writability_through<const enum_t(*)[3]>, self);

    MAGIC_CHECK(writability_through<enum_t[3][3]>, self);
    MAGIC_CHECK(writability_through<enum_t(*)[3][3]>, external);
    MAGIC_CHECK(writability_through<const enum_t(*)[3][3]>, self);

    MAGIC_CHECK(writability_through<nullptr_t[3]>, none);
    MAGIC_CHECK(writability_through<nullptr_t(*)[3]>, self);
    MAGIC_CHECK(writability_through<const nullptr_t(*)[3]>, self);

    MAGIC_CHECK(writability_through<nullptr_t[3][3]>, none);
    MAGIC_CHECK(writability_through<nullptr_t(*)[3][3]>, self);
    MAGIC_CHECK(writability_through<const nullptr_t(*)[3][3]>, self);
}

TEST(external_freeze, mutable_state_sharing_mode_specialization)
{
    using enum writability;

    MAGIC_CHECK(writability_through<wrapper<int>>, self);
    MAGIC_CHECK(writability_through<wrapper<const int>>, none);
    MAGIC_CHECK(writability_through<wrapper<int&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const int&>>, none);
    MAGIC_CHECK(writability_through<wrapper<int&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const int&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<int*>>, external);
    MAGIC_CHECK(writability_through<wrapper<int* const>>, external);
    MAGIC_CHECK(writability_through<wrapper<int*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<int* const&>>, external);
    MAGIC_CHECK(writability_through<wrapper<int*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<int* const&&>>, external);

    MAGIC_CHECK(writability_through<wrapper<const int*>>, self);
    MAGIC_CHECK(writability_through<wrapper<const int* const>>, none);
    MAGIC_CHECK(writability_through<wrapper<const int*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const int* const&>>, none);
    MAGIC_CHECK(writability_through<wrapper<const int*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const int* const&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<double>>, self);
    MAGIC_CHECK(writability_through<wrapper<const double>>, none);
    MAGIC_CHECK(writability_through<wrapper<double&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const double&>>, none);
    MAGIC_CHECK(writability_through<wrapper<double&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const double&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<double*>>, external);
    MAGIC_CHECK(writability_through<wrapper<double* const>>, external);
    MAGIC_CHECK(writability_through<wrapper<double*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<double* const&>>, external);
    MAGIC_CHECK(writability_through<wrapper<double*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<double* const&&>>, external);

    MAGIC_CHECK(writability_through<wrapper<const double*>>, self);
    MAGIC_CHECK(writability_through<wrapper<const double* const>>, none);
    MAGIC_CHECK(writability_through<wrapper<const double*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const double* const&>>, none);
    MAGIC_CHECK(writability_through<wrapper<const double*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const double* const&&>>, none);

    enum enum_t {
        first,
        second
    };

    MAGIC_CHECK(writability_through<wrapper<enum_t>>, self);
    MAGIC_CHECK(writability_through<wrapper<const enum_t>>, none);
    MAGIC_CHECK(writability_through<wrapper<enum_t&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t&>>, none);
    MAGIC_CHECK(writability_through<wrapper<enum_t&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<enum_t*>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t* const>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t* const&>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t* const&&>>, external);

    MAGIC_CHECK(writability_through<wrapper<const enum_t*>>, self);
    MAGIC_CHECK(writability_through<wrapper<const enum_t* const>>, none);
    MAGIC_CHECK(writability_through<wrapper<const enum_t*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t* const&>>, none);
    MAGIC_CHECK(writability_through<wrapper<const enum_t*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t* const&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<nullptr_t>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t&>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t&>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t&&>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<nullptr_t*>>, self);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t* const>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t* const&>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t* const&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<const nullptr_t*>>, self);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t* const>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t*&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t* const&>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t*&&>>, external);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t* const&&>>, none);

    MAGIC_CHECK(writability_through<wrapper<enum_t[3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<const enum_t[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<enum_t(&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t(&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<enum_t(&&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t(&&)[3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<enum_t (*)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (* const)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (*&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (* const&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (*&&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (* const&&)[3]>>, external);

    MAGIC_CHECK(writability_through<wrapper<const enum_t (*)[3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (* const)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (*&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (* const&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (*&&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (* const&&)[3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<enum_t[3][3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<const enum_t[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<enum_t(&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t(&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<enum_t(&&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t(&&)[3][3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<enum_t (*)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (* const)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (*&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (* const&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (*&&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<enum_t (* const&&)[3][3]>>, external);

    MAGIC_CHECK(writability_through<wrapper<const enum_t (*)[3][3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (* const)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (*&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (* const&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (*&&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const enum_t (* const&&)[3][3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<nullptr_t[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t(&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t(&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t(&&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t(&&)[3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<nullptr_t (*)[3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (* const)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (*&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (* const&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (*&&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (* const&&)[3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (*)[3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (* const)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (*&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (* const&)[3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (*&&)[3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (* const&&)[3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<nullptr_t[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t(&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t(&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t(&&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t(&&)[3][3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<nullptr_t (*)[3][3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (* const)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (*&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (* const&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (*&&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<nullptr_t (* const&&)[3][3]>>, none);

    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (*)[3][3]>>, self);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (* const)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (*&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (* const&)[3][3]>>, none);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (*&&)[3][3]>>, external);
    MAGIC_CHECK(writability_through<wrapper<const nullptr_t (* const&&)[3][3]>>, none);
}

TEST(external_freeze, writability_through_aggregate)
{
    struct unknown_t { int* p; };
    using enum writability;

    // empty aggregate
    MAGIC_CHECK((writability_through_aggregate<>), none);

    // std::nullptr_t: no mutable state
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<const std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t&>), none);
    MAGIC_CHECK((writability_through_aggregate<const std::nullptr_t&>), none);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t&&>), none);
    MAGIC_CHECK((writability_through_aggregate<const std::nullptr_t&&>), none);

    // int: self mutable state
    MAGIC_CHECK((writability_through_aggregate<int>), self);
    MAGIC_CHECK((writability_through_aggregate<const int>), none);
    MAGIC_CHECK((writability_through_aggregate<int&>), external);
    MAGIC_CHECK((writability_through_aggregate<const int&>), none);
    MAGIC_CHECK((writability_through_aggregate<int&&>), external);
    MAGIC_CHECK((writability_through_aggregate<const int&&>), none);

    // unknown_t: default external mutable state
    MAGIC_CHECK((writability_through_aggregate<unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t&>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&&>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t&&>), external);

    // none combined with none
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, const std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t&, const std::nullptr_t&>), none);

    // self combined with none
    MAGIC_CHECK((writability_through_aggregate<int, std::nullptr_t>), self);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, int>), self);
    MAGIC_CHECK((writability_through_aggregate<int, const std::nullptr_t>), self);
    MAGIC_CHECK((writability_through_aggregate<const int, std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<const int, const std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<const int&, std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<const int&&, std::nullptr_t>), none);

    // self combined with self
    MAGIC_CHECK((writability_through_aggregate<int, int>), self);
    MAGIC_CHECK((writability_through_aggregate<int, const int>), self);
    MAGIC_CHECK((writability_through_aggregate<const int, int>), self);
    MAGIC_CHECK((writability_through_aggregate<const int, const int>), none);

    // reference-to-self becomes external when non-const
    MAGIC_CHECK((writability_through_aggregate<int&>), external);
    MAGIC_CHECK((writability_through_aggregate<int&&>), external);
    MAGIC_CHECK((writability_through_aggregate<int, int&>), external);
    MAGIC_CHECK((writability_through_aggregate<int, int&&>), external);
    MAGIC_CHECK((writability_through_aggregate<const int, int&>), external);
    MAGIC_CHECK((writability_through_aggregate<const int&, int>), self);
    MAGIC_CHECK((writability_through_aggregate<const int&&, int>), self);
    MAGIC_CHECK((writability_through_aggregate<const int&, const int>), none);
    MAGIC_CHECK((writability_through_aggregate<const int&&, const int>), none);

    // external dominates none
    MAGIC_CHECK((writability_through_aggregate<unknown_t, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t&, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&&, std::nullptr_t>), external);

    // external dominates self
    MAGIC_CHECK((writability_through_aggregate<unknown_t, int>), external);
    MAGIC_CHECK((writability_through_aggregate<int, unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t, int>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t&, int>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&, int>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&&, int>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t, const int>), external);
    MAGIC_CHECK((writability_through_aggregate<const int, unknown_t>), external);

    // external combined with external
    MAGIC_CHECK((writability_through_aggregate<unknown_t, unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t, const unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const unknown_t, const unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t&, const unknown_t&>), external);

    // three-member combinations
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, std::nullptr_t, std::nullptr_t>), none);
    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, int, std::nullptr_t>), self);
    MAGIC_CHECK((writability_through_aggregate<const int, int, const int>), self);
    MAGIC_CHECK((writability_through_aggregate<const int, const int, const int>), none);

    MAGIC_CHECK((writability_through_aggregate<std::nullptr_t, int, unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<unknown_t, int, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<int, unknown_t, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const int, const std::nullptr_t, unknown_t>), external);
    MAGIC_CHECK((writability_through_aggregate<int&, const int&, std::nullptr_t>), external);
    MAGIC_CHECK((writability_through_aggregate<const int&, const int&&, std::nullptr_t>), none);
}

TEST(external_freeze, alias_frozen_default)
{
    MAGIC_CHECK(external_frozen<int>, true);
    MAGIC_CHECK(external_frozen<const int>, true);
    MAGIC_CHECK(external_frozen<int&>, false);
    MAGIC_CHECK(external_frozen<const int&>, true);
    MAGIC_CHECK(external_frozen<int&&>, false);
    MAGIC_CHECK(external_frozen<const int&&>, true);

    MAGIC_CHECK(external_frozen<double>, true);
    MAGIC_CHECK(external_frozen<const double>, true);
    MAGIC_CHECK(external_frozen<double&>, false);
    MAGIC_CHECK(external_frozen<const double&>, true);
    MAGIC_CHECK(external_frozen<double&&>, false);
    MAGIC_CHECK(external_frozen<const double&&>, true);
    
    enum enum_t{
        first,
        second
    };

    MAGIC_CHECK(external_frozen<enum_t>, true);
    MAGIC_CHECK(external_frozen<const enum_t>, true);
    MAGIC_CHECK(external_frozen<enum_t&>, false);
    MAGIC_CHECK(external_frozen<const enum_t&>, true);
    MAGIC_CHECK(external_frozen<enum_t&&>, false);
    MAGIC_CHECK(external_frozen<const enum_t&&>, true);

    MAGIC_CHECK(external_frozen<nullptr_t>, true);
    MAGIC_CHECK(external_frozen<const nullptr_t>, true);
    MAGIC_CHECK(external_frozen<nullptr_t&>, true);
    MAGIC_CHECK(external_frozen<const nullptr_t&>, true);
    MAGIC_CHECK(external_frozen<nullptr_t&&>, true);
    MAGIC_CHECK(external_frozen<const nullptr_t&&>, true);
}

TEST(external_freeze, alias_frozen_specializations)
{
    MAGIC_CHECK(external_frozen<wrapper<int>>, true);
    MAGIC_CHECK(external_frozen<wrapper<const int>>, true);
    MAGIC_CHECK(external_frozen<wrapper<int&>>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int&>>, true);
    MAGIC_CHECK(external_frozen<wrapper<int&&>>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int&&>>, true);

    MAGIC_CHECK(external_frozen<const wrapper<int>>, true);
    MAGIC_CHECK(external_frozen<const wrapper<const int>>, true);
    MAGIC_CHECK(external_frozen<const wrapper<int&>>, false);
    MAGIC_CHECK(external_frozen<const wrapper<const int&>>, true);
    MAGIC_CHECK(external_frozen<const wrapper<int&&>>, false);
    MAGIC_CHECK(external_frozen<const wrapper<const int&&>>, true);

    MAGIC_CHECK(external_frozen<wrapper<int>&>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int>&>, true);
    MAGIC_CHECK(external_frozen<wrapper<int&>&>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int&>&>, true);
    MAGIC_CHECK(external_frozen<wrapper<int&&>&>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int&&>&>, true);

    MAGIC_CHECK(external_frozen<const wrapper<int>&>, true);
    MAGIC_CHECK(external_frozen<const wrapper<const int>&>, true);
    MAGIC_CHECK(external_frozen<const wrapper<int&>&>, false);
    MAGIC_CHECK(external_frozen<const wrapper<const int&>&>, true);
    MAGIC_CHECK(external_frozen<const wrapper<int&&>&>, false);
    MAGIC_CHECK(external_frozen<const wrapper<const int&&>&>, true);

    MAGIC_CHECK(external_frozen<wrapper<int>&&>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int>&&>, true);
    MAGIC_CHECK(external_frozen<wrapper<int&>&&>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int&>&&>, true);
    MAGIC_CHECK(external_frozen<wrapper<int&&>&&>, false);
    MAGIC_CHECK(external_frozen<wrapper<const int&&>&&>, true);

    MAGIC_CHECK(external_frozen<const wrapper<int>&&>, true);
    MAGIC_CHECK(external_frozen<const wrapper<const int>&&>, true);
    MAGIC_CHECK(external_frozen<const wrapper<int&>&&>, false);
    MAGIC_CHECK(external_frozen<const wrapper<const int&>&&>, true);
    MAGIC_CHECK(external_frozen<const wrapper<int&&>&&>, false);
    MAGIC_CHECK(external_frozen<const wrapper<const int&&>&&>, true);
}

TEST(external_freeze, none)
{
    MAGIC_TCHECK(decltype(senluo::external_freeze<nullptr_t>(nullptr)), nullptr_t);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const nullptr_t>(nullptr)), nullptr_t);

    nullptr_t nullptr_value;

    MAGIC_TCHECK(decltype(senluo::external_freeze<nullptr_t&>(nullptr_value)), nullptr_t&);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const nullptr_t&>(nullptr_value)), const nullptr_t&);

    MAGIC_TCHECK(decltype(senluo::external_freeze<nullptr_t&&>(nullptr)), nullptr_t&&);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const nullptr_t&&>(nullptr)), const nullptr_t&&);
}

TEST(external_freeze, self)
{
    MAGIC_TCHECK(decltype(senluo::external_freeze<int>(3)), int);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const int>(3)), int);

    int value = 3;

    MAGIC_TCHECK(decltype(senluo::external_freeze<int&>(value)), const int&);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const int&>(value)), const int&);

    MAGIC_TCHECK(decltype(senluo::external_freeze<int&&>(3)), const int&&);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const int&&>(3)), const int&&);
}

TEST(external_freeze, default_shared)
{
    struct unknown_type{ int* p; };

    MAGIC_TCHECK(decltype(senluo::external_freeze<unknown_type>({})), unknown_external_frozen_type_of<unknown_type>);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const unknown_type>({})), unknown_external_frozen_type_of<const unknown_type>);

    unknown_type value;

    MAGIC_TCHECK(decltype(senluo::external_freeze<unknown_type&>(value)), unknown_external_frozen_type_of<unknown_type&>);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const unknown_type&>(value)), unknown_external_frozen_type_of<const unknown_type&>);

    MAGIC_TCHECK(decltype(senluo::external_freeze<unknown_type&&>({})), unknown_external_frozen_type_of<unknown_type&&>);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const unknown_type&&>({})), unknown_external_frozen_type_of<const unknown_type&&>);
}

TEST(external_freeze, specialization)
{
    int value = 3;
    wrapper<int&> ref_box{ value };

    MAGIC_TCHECK(decltype(senluo::external_freeze<wrapper<int&>>(std::move(ref_box))), wrapper<const int&>);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const wrapper<int&>>(std::move(ref_box))), wrapper<const int&>);

    MAGIC_TCHECK(decltype(senluo::external_freeze<wrapper<int&>&>(ref_box)), wrapper<const int&>);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const wrapper<int&>&>(ref_box)), wrapper<const int&>);

    MAGIC_TCHECK(decltype(senluo::external_freeze<wrapper<int&>&&>(std::move(ref_box))), wrapper<const int&>);
    MAGIC_TCHECK(decltype(senluo::external_freeze<const wrapper<int&>&&>(std::move(ref_box))), wrapper<const int&>);
}