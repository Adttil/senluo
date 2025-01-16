#ifndef RUZHOUXIE_ASTRICT_HPP
#define RUZHOUXIE_ASTRICT_HPP

#include "../tree.hpp"
#include "../general.hpp"
#include "principle.hpp"
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
                return senluo::make_tuple(senluo::merge_stricture_tree(tree1 | subtree<I>, tree2)...);
            }
            else
            {
                return senluo::make_tuple(senluo::merge_stricture_tree(tree1 | subtree<I>, tree2 | subtree<I>)...);
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

namespace senluo 
{
    template<typename TBasePrinciple, auto FoldedStrictureTree>
    struct astrict_principle : based_on<TBasePrinciple>, principle_interface<astrict_principle<TBasePrinciple, FoldedStrictureTree>>
    {
        constexpr decltype(auto) data(this auto&& self)
        {
            return FWD(self, base).data();
        }

        static constexpr auto layout()
        {
            return TBasePrinciple::layout();
        }
        
        static constexpr auto stricture_tree()
        { 
            return senluo::merge_stricture_tree(TBasePrinciple::stricture_tree(), FoldedStrictureTree);
        }

        static constexpr auto operation_tree()
        {
            return operation_t::none;
        }
    };

    template<typename T, auto FoldedStrictureTree>
    struct astrict_tree : based_on<T>, standard_interface<astrict_tree<T, FoldedStrictureTree>>
    {
        template<size_t I, typename Self> 
        constexpr decltype(auto) get(this Self&& self)
        {
            constexpr auto stricture_subtree = tag_tree_get<I>(FoldedStrictureTree);
            if constexpr(I >= size<T>)
            {
                return end();
            }
            else if constexpr(not std::same_as<decltype(stricture_subtree), const stricture_t>)
            {
                return astrict_tree<decltype(FWD(self, base) | subtree<I>), stricture_subtree>{ FWD(self, base) | subtree<I> };
            }
            else if constexpr(stricture_subtree == stricture_t::none || only_input<decltype(FWD(self, base) | subtree<I>)>())
            {
                return FWD(self, base) | subtree<I>;
            }
            else if constexpr(std::is_reference_v<decltype(FWD(self, base) | subtree<I>)> 
                              && only_input<decltype(senluo::to_readonly(FWD(self, base) | subtree<I>))>())
            {
                return senluo::to_readonly(FWD(self, base) | subtree<I>);
            }
            else
            {
                return astrict_tree<decltype(FWD(self, base) | subtree<I>), stricture_t::readonly>{ FWD(self, base) | subtree<I> };
            }
        }

        template<auto UsageTree, bool NoCopy, typename Self>
        constexpr auto principle(this Self&& self)
        {
            using base_principle_t = decltype(FWD(self, base) | senluo::principle<UsageTree, NoCopy>);
            if constexpr(equal(base_principle_t::operation_tree(), operation_t::none))
            {
                return astrict_principle<base_principle_t, FoldedStrictureTree>{ FWD(self, base) | senluo::principle<UsageTree, NoCopy> };
            }
            else
            {
                using base_plain_principle_t = plain_principle<decltype(FWD(self, base) | plainize<UsageTree>)>;
                
                return astrict_principle<base_plain_principle_t, FoldedStrictureTree>{ 
                    base_plain_principle_t{ FWD(self, base) | plainize<UsageTree> }
                };
            }
        }
    };

    namespace detail
    {
        template<auto FoldedStrictureTree>
        struct astrict_t : adaptor_closure<astrict_t<FoldedStrictureTree>>
        {
            template<typename T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                if constexpr(not std::same_as<decltype(FoldedStrictureTree), stricture_t>)
                {
                    return astrict_tree<senluo::unwrap_t<T>, FoldedStrictureTree>{ unwrap_fwd(FWD(t)) };
                }
                else if constexpr(FoldedStrictureTree == stricture_t::none || only_input<T>())
                {
                    return decltype(wrap(FWD(t))){ unwrap_fwd(FWD(t)) };
                }
                else if constexpr(std::is_reference_v<T> && only_input<decltype(senluo::to_readonly(FWD(t)))>())
                {
                    return decltype(wrap(senluo::to_readonly(FWD(t)))){ unwrap_fwd(senluo::to_readonly(FWD(t))) };
                }
                else
                {
                    return astrict_tree<senluo::unwrap_t<T>, stricture_t::readonly>{ unwrap_fwd(FWD(t)) };
                }
            }
        };
    }

    template<auto StrictureTree>
    inline constexpr detail::astrict_t<fold_tag_tree<StrictureTree>()> astrict{};
}

#include "../macro_undef.hpp"
#endif