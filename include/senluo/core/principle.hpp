#ifndef SENLUO_PRINCIPLE_HPP
#define SENLUO_PRINCIPLE_HPP

#include "../tools/general.hpp"
#include "subtree.hpp"
#include "wrap.hpp"
#include "tag.hpp"
#include "pretreat.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    struct null_principle
    {
        constexpr std::in_place_t data() noexcept
        {
            return std::in_place_t{};
        }
        
        static constexpr auto layout = indexes_of_whole;
        static constexpr auto stricture_tree = stricture_t::none;
        static constexpr size_t operation_tree_count = 0uz;
        
        //static constexpr auto independence_tree(){ return independence_t::safe; }
    };

    template<class T>
    struct plain_principle
    {
        T&& value;

        // template<class Self>
        // constexpr decltype(auto) data(this Self&& self)
        // {
        //     if constexpr(std::is_object_v<Self> && std::is_object_v<T>)
        //     {
        //         return senluo::decay_copy(std::move(self.value));
        //     }
        //     else
        //     {
        //         return FWD(((Self&&)self), value);
        //     }
        // }

        //gcc workround
        template<class Self>
        constexpr auto&& data(this Self&& self)
        {
            return FWD(self, value);
        }

        template<class Self> requires(std::is_object_v<Self> && std::is_object_v<T>)
        constexpr auto data(this Self&& self)
        {
            return senluo::decay_copy(std::move(self.value));
        }
        
        static constexpr auto layout = indexes_of_whole;
        static constexpr auto stricture_tree = stricture_t::none;
        static constexpr size_t operation_tree_count = 0uz;

        //static consteval auto independence_tree(){ return independence_t::safe; }
    };

    template<class T, auto UsageTree>
    struct trivial_principle
    {        
        T&& value_;

        constexpr decltype(auto) value()
        {
            if constexpr(std::is_rvalue_reference_v<T>)
            {
                return wrapper<T>{ (T)value_ };
            }
            else
            {
                return (T)value_;
            }
        }

        constexpr auto data()&&
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype((value() | tree_get<I> | principle<detail::tag_tree_get<I>(UsageTree)>).data())...>
                {
                    (value() | tree_get<I> | principle<detail::tag_tree_get<I>(UsageTree)>).data()...
                };
            }(std::make_index_sequence<size<T>>{});
        }

        static constexpr auto layout = []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(
                detail::layout_add_prefix(principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::layout, array{ I })...
            );
        }(std::make_index_sequence<size<T>>{});

        static constexpr auto stricture_tree = []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(
                principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::stricture_tree...
            );
        }(std::make_index_sequence<size<T>>{});

        static constexpr size_t operation_tree_count = []<size_t...I>(std::index_sequence<I...>)
        {
            return std::max({
                principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::operation_tree_count...
            });
        }(std::make_index_sequence<size<T>>{});

        template<size_t I>
        static constexpr auto operation_tree = []<size_t...J>(std::index_sequence<J...>)
        {
            return make_tuple(
                detail::principle_operation_tree<principle_t<subtree_t<T, J>, detail::tag_tree_get<J>(UsageTree)>, I>()...
            );
        }(std::make_index_sequence<size<T>>{});

        // static consteval auto independence_tree()
        // {
        //     return []<size_t...I>(std::index_sequence<I...>)
        //     {
        //         return make_tuple(
        //             principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::independence_tree()...
        //         );
        //     }(std::make_index_sequence<size<T>>{});
        // }
    };

    template<class T, auto UsageTree>
    concept plain = std::same_as<principle_t<T, UsageTree>, plain_principle<unwrap_t<T>>>;

    template<auto UsageTree>
    struct detail::principle_fn_ns::principle_fn : adaptor_closure<principle_fn<UsageTree>>
    {
        // Complex sfinae and noexcept are not currently provided.
        template<typename T>
        constexpr auto operator()(T&& tree)const
        {
            if constexpr(detail::equal(UsageTree, usage_t::none))
            {
                return null_principle{};
            }
            else if constexpr(requires{ principle<UsageTree>(FWD(tree)); })
            {
                return principle<UsageTree>(FWD(tree));
            }
            else if constexpr(terminal<T>)
            {                
                return plain_principle<unwrap_t<T>>{ unwrap_fwd(FWD(tree)) };
            }
            else return[&]<size_t...I>(std::index_sequence<I...>)
            {
                if constexpr((... && plain<tree_get_t<I, T>, detail::tag_tree_get<I>(UsageTree)>))
                {
                    return plain_principle<unwrap_t<T>>{ unwrap_fwd((T&&)(tree)) };
                }
                else
                {
                    //static_assert(std::same_as<decltype(tree), T&&>);
                    return trivial_principle<unwrap_t<T&&>, UsageTree>(unwrap_fwd((T&&)tree));
                }
            }(std::make_index_sequence<size<T>>{});
        }
    };

    namespace detail
    {
        template<auto UsageTree, size_t I, template<class...> class Tpl, class T>
        constexpr decltype(auto) subplainized_pretreated_unchecked(T&& t)
        {
            if constexpr(detail::equal(UsageTree, usage_t::none))
            {
                return tuple{};
            }
            else if constexpr(terminal<tree_get_t<I, T>>)
            {
                return tree_get<I>(FWD(t));
            }
            else return [&]<size_t...J>(std::index_sequence<J...>)
            {
                return tuple<decltype(detail::subplainized_pretreated_unchecked<detail::tag_tree_get<J>(UsageTree), J, Tpl>(subtree<I>(FWD(t))))...>
                {
                    detail::subplainized_pretreated_unchecked<detail::tag_tree_get<J>(UsageTree), J, Tpl>(subtree<I>(FWD(t)))...
                };
            }(std::make_index_sequence<size<tree_get_t<I, T>>>{});
        }

        template<auto UsageTree, template<class...> class Tpl, class T>
        constexpr decltype(auto) plainized_pretreated_unchecked(T&& t)
        {
            if constexpr(detail::equal(UsageTree, usage_t::none))
            {
                return tuple{};
            }
            else if constexpr(terminal<T>)
            {
                return (T)FWD(t);
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype(detail::subplainized_pretreated_unchecked<detail::tag_tree_get<I>(UsageTree), I, Tpl>(FWD(t)))...>
                {
                    detail::subplainized_pretreated_unchecked<detail::tag_tree_get<I>(UsageTree), I, Tpl>(FWD(t))...
                };
            }(std::make_index_sequence<size<T>>{});
        }

        template<auto UsageTree, template<class...> class Tpl = tuple, class T>
        constexpr decltype(auto) plainized_unchecked(T&& t)
        {
            return plainized_pretreated_unchecked<UsageTree, Tpl>(FWD(t) | sequence_by_usage<UsageTree>);
        }

        template<auto FoldedUsageTree, template<class...> class Tpl>
        struct plainize_fn : adaptor_closure<plainize_fn<FoldedUsageTree, Tpl>>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& tree) const
            {
                constexpr auto usage = detail::unfold_usage_when_used<FoldedUsageTree, shape_t<T>>();
                return detail::plainized_pretreated_unchecked<usage, Tpl>(FWD(tree));
            }
        };
    }
    
    inline namespace functors 
    {
        template<auto UsageTree = usage_t::once, template<class...> class Tpl = tuple>
        inline constexpr detail::plainize_fn<detail::fold_usage_when_unused<UsageTree>(), Tpl> plainize{};
    }
}

#include "../tools/macro_undef.hpp"
#endif