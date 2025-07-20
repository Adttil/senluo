#ifndef SENLUO_ASTRICT_HPP
#define SENLUO_ASTRICT_HPP

#include "../tools/general.hpp"
#include "../tools/adaptor.hpp"
#include "subtree.hpp"
#include "make.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    enum class stricture_t
    {
        none,
        readonly
    };
}

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

    template<size_t I, class T>
    constexpr auto stricture_get(const T& tag_tree)
    {
        if constexpr(std::same_as<T, stricture_t>)
        {
            return tag_tree;
        }
        else
        {
            return get<I>(tag_tree);
        }
    }

    template<auto TagTree>
    constexpr auto fold_stricture()
    {
        if constexpr(terminal<decltype(TagTree)>)
        {
            return TagTree;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subresults = make_tuple(detail::fold_stricture<subtree<I>(TagTree)>()...);
            if constexpr((terminal<decltype(get<0uz>(subresults))> && ... && detail::equal(get<I>(subresults), get<0uz>(subresults))))
            {
                return get<0uz>(subresults);
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<decltype(TagTree)>>{});
    }
}

namespace senluo::detail
{
    template<class T, auto StrictureTree>
    struct astrict_tree
    {
        T base;

        template<size_t I, class Self> 
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            constexpr auto substricture = detail::stricture_get<I>(StrictureTree);
            if constexpr(not std::same_as<decltype(substricture), const stricture_t>)
            {
                return astrict_tree<decltype(FWD(self, base) | subtree<I>), substricture>{
                    senluo::tree_get<I>(FWD(self, base))
                };
            }
            else if constexpr(substricture == stricture_t::none || detail::only_input<decltype(senluo::tree_get<I>(FWD(self, base)))>())
            {
                return senluo::tree_get<I>(FWD(self, base));
            }
            else if constexpr(detail::only_input<decltype(senluo::tree_get<I>(detail::to_readonly(FWD(self, base))))>())
            {
                return senluo::tree_get<I>(detail::to_readonly(FWD(self, base)));
            }
            else if constexpr(std::is_rvalue_reference_v<decltype(senluo::tree_get<I>(detail::to_readonly(FWD(self, base))))> 
                              && detail::only_input<decltype(detail::to_readonly(senluo::tree_get<I>(detail::to_readonly(FWD(self, base)))))>())
            {
                return detail::to_readonly(senluo::tree_get<I>(detail::to_readonly(FWD(self, base))));
            }
            else
            {
                return astrict_tree<decltype(FWD(self, base) | senluo::subtree<I>), stricture_t::readonly>{ 
                    senluo::tree_get<I>(FWD(self, base)) 
                };
            }
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return size<T>;
        }

        template<class U>
        static constexpr astrict_tree make_from(U&& u, custom_t = {})
        AS_EXPRESSION(
            astrict_tree{ FWD(u) | make<T> }
        )
    };

    template<auto FoldedStrictureTree, class T>
    constexpr decltype(auto) astrict_unchecked(T&& t)
    {
        if constexpr(not std::same_as<decltype(FoldedStrictureTree), stricture_t>)
        {
            return astrict_tree<pass_t<T>, FoldedStrictureTree>{ FWD(t) };
        }
        else if constexpr(FoldedStrictureTree == stricture_t::none || detail::only_input<pass_t<T>>())
        {
            return pass(FWD(t));
        }
        else if constexpr(std::is_reference_v<T> && detail::only_input<decltype(detail::to_readonly(t))>())
        {
            return detail::to_readonly(t);
        }
        else
        {
            return astrict_tree<pass_t<T>, stricture_t::readonly>{ FWD(t) };
        }
    }
    
    template<auto FoldedStrictureTree>
    struct astrict_fn : adaptor_closure<astrict_fn<FoldedStrictureTree>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t) const
        {
            return detail::astrict_unchecked<FoldedStrictureTree>(FWD(t));
        }
    };
}

namespace senluo 
{
    inline namespace functors
    {
        template<auto StrictureTree>
        inline constexpr detail::astrict_fn<detail::fold_stricture<StrictureTree>()> astrict{};
    }
}

#include "../tools/macro_undef.hpp"
#endif