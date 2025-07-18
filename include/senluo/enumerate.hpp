#ifndef SENLUO_ENUMERATE_HPP
#define SENLUO_ENUMERATE_HPP

#include "tools/constant.hpp"
#include "tools/general.hpp"
#include "core.hpp"

#include "tools/macro_define.hpp"

namespace senluo
{
    template<auto Start_, auto End_>
    struct iota_view
    {
        template<size_t I, unwarp_derived_from<iota_view> Self>
        friend constexpr auto tree_get(Self&& self)
        {
            return constant_t<Start_ + I>{};
        }
    };

    template<auto Start_, auto End_>
    struct tree_size<iota_view<Start_, End_>>
    {
        static constexpr size_t value = static_cast<size_t>(End_ - Start_);
    };

    template<auto Start_, auto End_>
    constexpr iota_view<Start_, End_> iota()
    {
        return {};
    }

    namespace detail
    {
        struct enumerate_fn : tree_adaptor_closure<enumerate_fn>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& t)const noexcept
            {
                return zip(iota<0uz, size<T>>(), FWD(t));
            }
        };
    }

    inline namespace functor
    {
        inline constexpr detail::enumerate_fn enumerate{};
    }
}

#include "tools/macro_undef.hpp"
#endif