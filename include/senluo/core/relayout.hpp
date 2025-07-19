#ifndef SENLUO_RELAYOUT_HPP
#define SENLUO_RELAYOUT_HPP

#include "../tools/adaptor.hpp"
#include "../tools/constant.hpp"
#include "wrap.hpp"
#include "subtree.hpp"
#include "tag.hpp"
#include "principle.hpp"

#include "../tools/macro_define.hpp"

namespace senluo::detail
{
    template<class BasePrinciple, auto FoldedLayout>
    struct relayout_principle
    {
        BasePrinciple base;

        constexpr decltype(auto) data()&&
        {
            return std::move(base).data();
        }

        static constexpr auto layout = 
            detail::relayout_layout<FoldedLayout, BasePrinciple::layout, shape_t<decltype(std::move(base).data())>>();

        static constexpr auto stricture_tree = 
            detail::relayout_stricture_tree<FoldedLayout, BasePrinciple::stricture_tree>();

        static constexpr auto operation_tree_count = BasePrinciple::operation_tree_count;

        template<size_t I>
        static constexpr auto operation_tree = 
            detail::relayout_operation_tree<FoldedLayout, BasePrinciple::template operation_tree<I>>();
    };
}

namespace senluo
{
    

    template<class T, auto FoldedLayout>
    struct relayout_tree : based_on<T>
    {
        template<size_t I, unwarp_derived_from<relayout_tree> Self>
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            constexpr auto layout = detail::layout_get<I>(FoldedLayout);
            if constexpr(indexical<decltype(layout)>)
            {
                return FWD(self).unwrap_base() | subtree<detail::layout_get<I>(FoldedLayout)>;
            }
            else
            {
                return relayout_tree<unwrap_t<decltype(FWD(self).unwrap_base())>, detail::layout_get<I>(FoldedLayout)>{
                    unwrap_fwd(FWD(self).unwrap_base()) 
                };
            }
        }
        
        template<auto UsageTree, unwarp_derived_from<relayout_tree> Self>
        friend constexpr auto principle(Self&& self)
        {
            constexpr auto base_usage = detail::fold_usage_when_unused<detail::inverse_relayout_usage_tree<FoldedLayout>(UsageTree, shape<T>)>();
            using base_principle_t = principle_t<decltype(FWD(self).unwrap_base()), base_usage>;

            return detail::relayout_principle<base_principle_t, FoldedLayout>{
                    principle<base_usage>(FWD(self).unwrap_base())
            };

            // if constexpr(detail::is_enable_to_relayout_operation_tree<FoldedLayout>(base_principle_t::operation_tree))
            // {
            //     return detail::relayout_principle<base_principle_t, FoldedLayout>{
            //         principle<base_usage>(FWD(self).unwrap_base())
            //     };
            // }
            // else
            // {
            //     using plain_principle_t = plain_principle<decltype(detail::plainized_unchecked<base_usage>(FWD(self).unwrap_base()))>;
            //     detail::relayout_principle<plain_principle_t, FoldedLayout>{
            //         plain_principle_t{ detail::plainized_unchecked<base_usage>(FWD(self).unwrap_base()) }
            //     };
            // }
        }        
    };

    template<class T, auto FoldedLayout>
    struct tree_size<relayout_tree<T, FoldedLayout>>
    {
        static constexpr size_t value = []
        {
            if constexpr(detail::indexical_array<decltype(FoldedLayout)>)
            {
                return size<subtree_t<T, FoldedLayout>>;
            }
            else
            {
                return std::tuple_size_v<std::remove_const_t<decltype(FoldedLayout)>>;
            }
        }();
    };

    namespace detail 
    {
        template<auto Layout>
        constexpr auto layout_used_indexes()
        {
            if constexpr(indexical_array<decltype(Layout)>)
            {
                return tuple{ Layout };
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple_cat(detail::layout_used_indexes<get<I>(Layout)>()...); 
            }(std::make_index_sequence<std::tuple_size_v<decltype(Layout)>>{});
        }

        template<auto FoldedLayout, class T>
        constexpr decltype(auto) relayout_unchecked(T&& t)
        {
            if constexpr(detail::indexical_array<decltype(FoldedLayout)>)
            {
                if constexpr(std::is_rvalue_reference_v<decltype(FWD(t) | subtree<FoldedLayout>)>)
                {
                    return wrapper<decltype(FWD(t) | subtree<FoldedLayout>)>{ FWD(t) | subtree<FoldedLayout> };
                }
                else
                {
                    return FWD(t) | subtree<FoldedLayout>;
                }
            }
            else
            {
                return relayout_tree<unwrap_t<T>, FoldedLayout>{
                    unwrap_fwd(FWD(t))
                };
            }
        }

        template<auto Layout>
        struct relayout_fn : adaptor_closure<relayout_fn<Layout>>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const noexcept
            {
                constexpr auto folded_layout = detail::fold_layout_v<Layout, shape_t<T>>;
                return detail::relayout_unchecked<folded_layout>(FWD(t));
            }

            template<class S>
            static constexpr relayout_fn<detail::replicate<S>(Layout)> replicate(S = {})noexcept
            {
                return {};
            }

            template<auto Indexes>
            friend constexpr auto operator/(constant_t<Indexes>, relayout_fn)
            {
                return []<size_t...I>(std::index_sequence<I...>)
                {
                    return constant_t<tuple_cat(detail::layout_used_indexes<detail::sublayout<get<I>(Indexes)>(Layout)>()...)>{};
                }(std::make_index_sequence<std::tuple_size_v<decltype(Indexes)>>{});
            }
        };
    }

    inline namespace functors
    {
        template<auto Layout>
        inline constexpr detail::relayout_fn<Layout> relayout{};
    }
}

namespace senluo
{
    template<class Relayouter>
    struct relayouter_interface;

    template<class Relayouter>
    struct relayouter_interface : adaptor_closure<Relayouter>
    {
        template<class T>
        static consteval auto layout() noexcept
        {
            constexpr auto tree = detail::default_unfolded_layout<T>();
            return detail::fold_layout_v<Relayouter::relayout(tree), shape_t<T>>;
        }

        template<typename T>
        constexpr decltype(auto) operator()(T&& tree) const
        {
            constexpr auto layout = Relayouter::template layout<T>();
            return detail::relayout_unchecked<layout>(FWD(tree));
        }
    };

    namespace detail
    {
        template<size_t N>
        struct repeat_t : relayouter_interface<repeat_t<N>>
        {
            template<class T>
            static consteval auto layout() noexcept
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ (I, indexes_of_whole)... };
                }(std::make_index_sequence<N>{});
            }

            static constexpr auto relayout(const auto& tree)
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ (I, tree)... };
                }(std::make_index_sequence<N>{});
            }
        };

        template<class T>
        struct repeat_as_t : relayouter_interface<repeat_as_t<T>>
        {
            template<class U>
            static consteval auto layout() noexcept
            {
                return detail::replicate(indexes_of_whole, shape<T>);
            }
        };

        // template<array TensorShape>
        // struct repeat_by_t : relayouter_interface<repeat_by_t<TensorShape>>
        // {
        //     template<class T>
        //     static consteval auto layout() noexcept
        //     {
        //         //return detail::replicate(indexes_of_whole, shape<>);
        //     }
        // };
    }

    inline namespace functors
    {
        template<size_t N>
        inline constexpr detail::repeat_t<N> repeat{};
        
        template<class Tree>
        inline constexpr detail::repeat_as_t<Tree> repeat_as{};

        // template<array TensorShape>
        // inline constexpr detail::repeat_by_t<TensorShape> repeat_by{};
    }

    namespace detail
    {
        template<size_t I, size_t Axis>
        struct component_t : relayouter_interface<component_t<I, Axis>>
        {
            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                if constexpr (Axis == 0uz)
                {
                    static_assert(I < size<T>, "Component index out of range.");
                    return subtree<I>(tree);
                }
                else
                {
                    static_assert(branched<T>, "Axis index out of range.");
                    return[&]<size_t...J>(std::index_sequence<J...>)
                    {
                        return make_tuple(component_t<I, Axis - 1uz>::relayout(subtree<J>(tree))...);
                    }(std::make_index_sequence<size<T>>{});
                }
            }
        };
    }

    inline namespace functors
    {
        template<size_t I, size_t Axis>
        inline constexpr detail::component_t<I, Axis> component{}; 
    }

    namespace detail
    {
        template<size_t Axis1, size_t Axis2>
        struct transpose_t : relayouter_interface<transpose_t<Axis1, Axis2>>
        {
            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                if constexpr (Axis1 == 0uz)
                {
                    constexpr size_t N = tensor_shape<T>[Axis2];
                    return[&]<size_t...I>(std::index_sequence<I...>)
                    {
                        return senluo::make_tuple(component_t<I, Axis2>::relayout(tree)...);
                    }(std::make_index_sequence<N>{});
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return senluo::make_tuple(transpose_t<Axis1 - 1uz, Axis2 - 1uz>::relayout(subtree<I>(tree))...);
                }(std::make_index_sequence<size<T>>{});
            }
        };
    }

    inline namespace functors
    {
        template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
        inline constexpr detail::transpose_t<Axis1, Axis2> transpose{}; 
    }

    namespace detail
    {
        struct inverse_t : relayouter_interface<inverse_t>
        {
            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto last_index = size<T> - 1uz;
                    return make_tuple(subtree<last_index - I>(tree)...);
                }(std::make_index_sequence<size<T>>{});
            }

            template<class T>
            static consteval auto layout() noexcept
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto last_index = size<T> - 1uz;
                    return tuple{ array{ last_index - I }... };
                }(std::make_index_sequence<size<T>>{});
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::inverse_t inverse{}; 
    }

    namespace detail
    {
        template<size_t N>
        struct take_fn : relayouter_interface<take_fn<N>>
        {
            template<typename T>
            static consteval auto layout()
            {
                if constexpr(N == size<T>)
                {
                    return indexes_of_whole;
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, 1uz>{ I }... };
                }(std::make_index_sequence<N>{});
            }

            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(subtree<I>(tree)...);
                }(std::make_index_sequence<N>{});
            }
        };

        template<size_t N>
        struct drop_fn : relayouter_interface<drop_fn<N>>
        {
            template<typename T>
            static consteval auto layout()
            {
                if constexpr(N == 0uz)
                {
                    return indexes_of_whole;
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, 1uz>{ I + N }... };
                }(std::make_index_sequence<size<T> - N>{});
            }

            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(subtree<I + N>(tree)...);
                }(std::make_index_sequence<size<T> - N>{});
            }
        };

        template<size_t N, size_t M>
        struct drop_take_fn : relayouter_interface<drop_take_fn<N, M>>
        {
            template<typename T>
            static consteval auto layout()
            {
                if constexpr(N == 0uz && M == size<T>)
                {
                    return indexes_of_whole;
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, 1uz>{ I + N }... };
                }(std::make_index_sequence<M>{});
            }

            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(subtree<I + N>(tree)...);
                }(std::make_index_sequence<M>{});
            }
        };
    }

    inline namespace functors
    {
        template<size_t N>
        inline constexpr detail::take_fn<N> take{}; 

        template<size_t N>
        inline constexpr detail::drop_fn<N> drop{}; 

        template<size_t N, size_t M>
        inline constexpr detail::drop_take_fn<N, M> drop_take{}; 
    }

    namespace detail
    {
        struct combine_t
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<unwrap_t<T>...>{ unwrap_fwd(FWD(t))... };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::combine_t combine{};  
    }

    namespace detail
    {
        struct zip_t
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<T...>{ FWD(t)... } | transpose<>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::zip_t zip{};  
    }

    namespace detail
    {
        struct mat_zip_t
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<T...>{ FWD(t)... } | transpose<> | transpose<1, 2>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::mat_zip_t mat_zip{};  
    }
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_RELAYOUT_HPP