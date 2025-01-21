#ifndef RUZHOUXIE_ASTRICT_HPP
#define RUZHOUXIE_ASTRICT_HPP

#include "../general.hpp"
#include "principle.hpp"
#include "subtree.hpp"
#include "wrap.hpp"

#include "../macro_define.hpp"

namespace senluo 
{
    template<class S1, class S2>
    constexpr auto merge_stricture_tree(const S1& tree1, const S2& tree2)
    {
        if constexpr(std::same_as<S1, stricture_t>)
        {
            if constexpr(std::same_as<S2, stricture_t>)
            {
                return tree1 & tree2;
            }
            else
            {
                return senluo::merge_stricture_tree(tree2, tree1);
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        { 
            //static_assert(size<S1> > 0);
            if constexpr(std::same_as<S2, stricture_t>)
            {
                return senluo::make_tuple(senluo::merge_stricture_tree(get<I>(tree1), tree2)...);
            }
            else
            {
                return senluo::make_tuple(senluo::merge_stricture_tree(get<I>(tree1), get<I>(tree2))...);
            }
        }(std::make_index_sequence<size<S1>>{});
    }

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
            return (... && only_input<subtree_t<T, I>>());
        }(std::make_index_sequence<size<T>>{});
    }
}

namespace senluo::detail::astrict_ns
{
    template<typename TBasePrinciple, auto FoldedStrictureTree>
    struct principle_t : detail::based_on<TBasePrinciple>, principle_interface<principle_t<TBasePrinciple, FoldedStrictureTree>>
    {
        friend constexpr decltype(auto) data(unwarp_derived_from<principle_t> auto&& self)
        {
            return data(FWD(self) | base);
        }

        static consteval auto layout()
        {
            return TBasePrinciple::layout();
        }
        
        static consteval auto stricture_tree()
        { 
            return senluo::merge_stricture_tree(TBasePrinciple::stricture_tree(), FoldedStrictureTree);
        }

        static consteval auto operation_tree()
        {
            return operation_t::none;
        }
    };

    template<typename T, auto FoldedStrictureTree>
    struct tree_t : detail::based_on<T>, standard_interface<tree_t<T, FoldedStrictureTree>>
    {
        template<size_t I, unwarp_derived_from<tree_t> Self> 
        friend constexpr decltype(auto) subtree(Self&& self)
        {
            constexpr auto stricture_subtree = detail::tag_tree_get<I>(FoldedStrictureTree);
            if constexpr(I >= size<T>)
            {
                return end();
            }
            else if constexpr(not std::same_as<decltype(stricture_subtree), const stricture_t>)
            {
                return tree_t<decltype(FWD(self) | base | senluo::subtree<I>), stricture_subtree>{ FWD(self) | base | senluo::subtree<I> };
            }
            else if constexpr(stricture_subtree == stricture_t::none || only_input<decltype(FWD(self) | base | senluo::subtree<I>)>())
            {
                return FWD(self) | base | senluo::subtree<I>;
            }
            else if constexpr(std::is_reference_v<decltype(FWD(self) | base | senluo::subtree<I>)> 
                              && only_input<decltype(detail::to_readonly(detail::to_readonly(FWD(self)) | base | senluo::subtree<I>))>())
            {
                return detail::to_readonly(detail::to_readonly(FWD(self)) | base | senluo::subtree<I>);
            }
            else
            {
                return tree_t<decltype(FWD(self) | base | senluo::subtree<I>), stricture_t::readonly>{ FWD(self) | base | senluo::subtree<I> };
            }
        }

        template<auto UsageTree, unwarp_derived_from<tree_t> Self>
        friend constexpr auto principle(Self&& self)
        {
            using base_principle_t = decltype(FWD(self) | base | senluo::principle<UsageTree>);
            if constexpr(detail::equal(base_principle_t::folded_operation_tree(), operation_t::none))
            {
                return principle_t<base_principle_t, FoldedStrictureTree>{ FWD(self) | base | senluo::principle<UsageTree> };
            }
            else
            {
                using base_plain_principle_t = decltype(FWD(self) | base | plainize_principle<UsageTree>);
                
                return principle_t<base_plain_principle_t, FoldedStrictureTree>{ 
                    FWD(self) | base | plainize_principle<UsageTree>
                };
            }
        }
    };
}

namespace senluo 
{
    template<auto FoldedStrictureTree>
    struct detail::astrict_t : adaptor_closure<astrict_t<FoldedStrictureTree>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            if constexpr(not std::same_as<decltype(FoldedStrictureTree), stricture_t>)
            {
                return astrict_ns::tree_t<senluo::unwrap_t<T>, FoldedStrictureTree>{ unwrap_fwd(FWD(t)) };
            }
            else if constexpr(FoldedStrictureTree == stricture_t::none || only_input<T>())
            {
                return decltype(wrap(FWD(t))){ unwrap_fwd(FWD(t)) };
            }
            else if constexpr(std::is_reference_v<T> && only_input<decltype(detail::to_readonly(FWD(t)))>())
            {
                return decltype(wrap(detail::to_readonly(FWD(t)))){ unwrap_fwd(detail::to_readonly(FWD(t))) };
            }
            else
            {
                return astrict_ns::tree_t<senluo::unwrap_t<T>, stricture_t::readonly>{ unwrap_fwd(FWD(t)) };
            }
        }
    };
}

#include "../macro_undef.hpp"
#endif