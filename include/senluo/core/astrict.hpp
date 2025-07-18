#ifndef SENLUO_ASTRICT_HPP
#define SENLUO_ASTRICT_HPP

#include "../tools/general.hpp"
#include "../tools/adaptor.hpp"
#include "../tools/constant.hpp"
#include "principle.hpp"
#include "subtree.hpp"
#include "wrap.hpp"

#include "../tools/macro_define.hpp"

namespace senluo::detail
{
    template<class T>
    consteval bool only_input()
    {
        if constexpr(not (std::is_const_v<std::remove_reference_t<T>> || std::is_object_v<T>))
        {
            return false;
        }
        else if constexpr(terminal<T>)
        {
            return true;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return (... && detail::only_input<subtree_t<T, I>>());
        }(std::make_index_sequence<size<T>>{});
    }
}

namespace senluo
{
    template<typename T, auto FoldedStrictureTree>
    struct astrict_tree : based_on<T>
    {
        template<size_t I, unwarp_derived_from<astrict_tree> Self> 
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            constexpr auto stricture_subtree = detail::tag_tree_get<I>(FoldedStrictureTree);
            if constexpr(not std::same_as<decltype(stricture_subtree), const stricture_t>)
            {
                return astrict_tree<decltype(FWD(self).unwrap_base() | senluo::subtree<I>), stricture_subtree>{
                     FWD(self).unwrap_base() | senluo::subtree<I> 
                };
            }
            else if constexpr(stricture_subtree == stricture_t::none || detail::only_input<decltype(FWD(self).unwrap_base() | senluo::subtree<I>)>())
            {
                return FWD(self).unwrap_base() | senluo::subtree<I>;
            }
            else if constexpr(std::is_reference_v<decltype(FWD(self).unwrap_base() | senluo::subtree<I>)> 
                              && detail::only_input<decltype(detail::to_readonly(detail::to_readonly(FWD(self)).unwrap_base() | senluo::subtree<I>))>())
            {
                return detail::to_readonly(detail::to_readonly(FWD(self)).unwrap_base() | senluo::subtree<I>);
            }
            else
            {
                return astrict_tree<decltype(FWD(self).unwrap_base() | senluo::subtree<I>), stricture_t::readonly>{ FWD(self).unwrap_base() | senluo::subtree<I> };
            }
        }
    };
}

namespace senluo::detail 
{
    template<auto FoldedStrictureTree, class T>
    constexpr decltype(auto) astrict_unchecked(T&& t)
    {
        if constexpr(not std::same_as<decltype(FoldedStrictureTree), stricture_t>)
        {
            return astrict_tree<unwrap_t<T>, FoldedStrictureTree>{ unwrap_fwd(FWD(t)) };
        }
        else if constexpr(FoldedStrictureTree == stricture_t::none || detail::only_input<T>())
        {
            return pass(FWD(t));
            //return decltype(wrap(FWD(t))){ unwrap_fwd(FWD(t)) };
        }
        else if constexpr(std::is_reference_v<T> && detail::only_input<decltype(detail::to_readonly(unwrap(FWD(t))))>())
        {
            return decltype(wrap(detail::to_readonly(unwrap_fwd(FWD(t))))){ detail::to_readonly(unwrap_fwd(FWD(t))) };
        }
        else
        {
            return astrict_tree<unwrap_t<T>, stricture_t::readonly>{ unwrap_fwd(FWD(t)) };
        }
    }
}

namespace senluo 
{
    namespace detail
    {
        template<auto FoldedStrictureTree>
        struct astrict_fn : tree_adaptor_closure<astrict_fn<FoldedStrictureTree>>
        {
            template<typename T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                return detail::astrict_unchecked<FoldedStrictureTree>(FWD(t));
            }

            template<class S>
            static constexpr astrict_fn<detail::fold_tag_tree<detail::replicate<S>(FoldedStrictureTree)>()> replicate(S = {}) noexcept
            {
                return {};
            }

            template<auto Indexes>
            friend constexpr constant_t<Indexes> operator/(constant_t<Indexes>, astrict_fn)
            {
                return {};
            }
        };
    }

    inline namespace functors
    {
        template<auto StrictureTree>
        inline constexpr detail::astrict_fn<detail::fold_tag_tree<StrictureTree>()> astrict{};
    }
}

#include "../tools/macro_undef.hpp"
#endif