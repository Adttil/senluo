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
        constexpr auto data() noexcept
        {
            return std::in_place_t{};
        }
        
        static consteval auto layout(){ return indexes_of_whole; }
        
        static consteval auto stricture_tree(){ return stricture_t::none; }
        
        static consteval auto operation_tree(){ return operation_t::none; }

        static consteval auto independence_tree(){ return independence_t::safe; }
    };

    template<class T>
    struct plain_principle
    {
        T&& value;

        template<class Self>
        constexpr decltype(auto) data(this Self&& self)
        {
            if constexpr(std::is_object_v<Self> && std::is_object_v<T>)
            {
                return senluo::decay_copy(std::move(self.value));
            }
            else
            {
                return FWD(self, value);
            }
        }
        
        static consteval auto layout(){ return indexes_of_whole; }
        
        static consteval auto stricture_tree(){ return stricture_t::none; }
        
        static consteval auto operation_tree(){ return operation_t::none; }

        static consteval auto independence_tree(){ return independence_t::safe; }
    };

    template<class T, auto UsageTree>
    struct trivial_principle
    {        
        T&& value_;

        template<class Self>
        constexpr decltype(auto) value(this Self&& self)
        {
            if constexpr(std::is_object_v<Self> && std::is_object_v<T>)
            {
                return std::move(self.value);
            }
            else
            {
                return FWD(self, value) | refer;
            }
        }

        // Complex sfinae and noexcept are not currently provided.
        constexpr auto data(this auto&& self)
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype((FWD(self).value() | tree_get<I> | principle<detail::tag_tree_get<I>(UsageTree)>).data())...>
                {
                    (FWD(self).value() | tree_get<I> | principle<detail::tag_tree_get<I>(UsageTree)>).data()...
                };
            }(std::make_index_sequence<size<T>>{});
        }

        static consteval auto layout()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(
                    detail::layout_add_prefix(principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::layout(), array{ I })...
                );
            }(std::make_index_sequence<size<T>>{});
        }

        static consteval auto stricture_tree()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(
                    principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::stricture_tree()...
                );
            }(std::make_index_sequence<size<T>>{});
        }

        static consteval auto operation_tree()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(
                    principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::operation_tree()...
                );
            }(std::make_index_sequence<size<T>>{});
        }

        static consteval auto independence_tree()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(
                    principle_t<subtree_t<T, I>, detail::tag_tree_get<I>(UsageTree)>::independence_tree()...
                );
            }(std::make_index_sequence<size<T>>{});
        }
    };

    template<class T, auto UsageTree>
    concept plain = std::same_as<principle_t<T, UsageTree>, plain_principle<unwrap_t<T>>>;

    template<auto UsageTree>
    struct detail::principle_t_ns::principle_fn : adaptor_closure<principle_fn<UsageTree>>
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
                    return plain_principle<unwrap_t<T>>{ unwrap_fwd(FWD(tree)) };
                }
                else
                {
                    return trivial_principle<unwrap_t<T>, UsageTree>{ unwrap_fwd(FWD(tree)) };
                }
            }(std::make_index_sequence<size<T>>{});
        }
    };
}

#include "../tools/macro_undef.hpp"
#endif