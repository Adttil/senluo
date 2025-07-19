#ifndef SENLUO_OPERATE_HPP
#define SENLUO_OPERATE_HPP

#include "../tools/constant.hpp"
#include "subtree.hpp"
#include "principle.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    // namespace detail 
    // {
    //     struct apply_t : adaptor<apply_t>
    //     {
    //         // Complex sfinae and noexcept are not currently provided.
    //         template<class Args, class Fn>
    //         constexpr decltype(auto) adapt(Args&& args, Fn&& fn)const
    //         {
    //             return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
    //             {
    //                 return FWD(fn)(tree_get<I>(FWD(args))...);
    //             }(std::make_index_sequence<size<Args>>{});
    //         }
    //     };
        
    //     struct apply_invoke_t : adaptor_closure<apply_invoke_t>
    //     {
    //         // Complex sfinae and noexcept are not currently provided.
    //         template<class T>
    //         constexpr decltype(auto) operator()(T&& tree)const
    //         {
    //             return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
    //             {
    //                 decltype(auto) sep_tree = seperate(FWD(tree));
    //                 return tree_get<0uz>(FWD(sep_tree))(tree_get<I + 1uz>(FWD(sep_tree))...);
    //             }(std::make_index_sequence<size<T> - 1uz>{});
    //         }
    //     };
    // }

    // inline constexpr detail::apply_t apply{};

    // inline constexpr detail::apply_invoke_t apply_invoke{};
}

namespace senluo::detail
{
    // template<size_t I, class OperationTree>
    // constexpr auto operation_tree_get(const OperationTree& operation_tree)
    // {
    //     if constexpr(std::same_as<OperationTree, operation_t>)
    //     {
    //         return operation_tree;
    //     }
    //     else
    //     {
    //         return get<I>(operation_tree);
    //     }
    // }

    template<auto Indexes, auto OperationTree>
    constexpr auto operate_dependencies()
    {
        if constexpr(Indexes.size() == 0uz)
        {
            return indexes_of_whole;
        }
        else if constexpr(adaptor_closuroid<decltype(OperationTree)>)
        {
            return decltype(OperationTree)::template dependencies<Indexes>();
        }
        else
        {
            return detail::array_cat(
                array{Indexes[0]}, 
                detail::operate_dependencies<detail::array_drop<1uz>(Indexes), get<Indexes[0]>(OperationTree)>()
            ); 
        }
    }

    template<class OperationTree>
    constexpr auto fold_operation_tree(OperationTree op_tree)
    {
        if constexpr(adaptor_closuroid<OperationTree>)
        {
            return op_tree;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subresults = make_tuple(detail::fold_operation_tree(op_tree.template get<I>())...);
            using subtype = std::remove_const_t<decltype(subresults)>;
            if constexpr((... && std::same_as<std::tuple_element_t<I, subtype>, pass_fn>))
            {
                return pass_fn{};
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<OperationTree>>{});
    }
}

namespace senluo
{
    template<class Base, auto OperationTree>
    struct operate_principle
    {
        Base base;

        constexpr decltype(auto) data()&&
        {
            return std::move(base).data();
        }

        static consteval auto adaptor_closure()
        {
            return Base::adaptor_closure() | operate<OperationTree>;
        }
    };

    template<class T, auto OperationTree>
    struct operate_view
    {
        wrapper<T> base;

        template<size_t I, class Self> 
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            constexpr auto op_subtree = get<I>(OperationTree);
            if constexpr(requires{ std::tuple_size<std::remove_const<decltype(op_subtree)>>::value; })
            {
                return operate_view<decltype(unwrap_opt_refer<Self>(self.base) || senluo::tree_get<I>), op_subtree>
                {
                    unwrap_opt_refer<Self>(self.base) || senluo::tree_get<I>
                };
            }
            else if constexpr(adaptor_closuroid<std::remove_const<decltype(op_subtree)>>)
            {
                return unwrap_opt_refer<Self>(self.base) || op_subtree;
            }
            else
            {
                return op_subtree(std::forward_like<Self>(self.base).get());
            }
        }

        template<class Self>
        constexpr decltype(auto) materialize(this Self&& self, custom_t = {})
        {
            if constexpr(std::is_lvalue_reference_v<Self&&>)
            {
                return (self);
            }
            else
            {
                return operate_view<decltype(senluo::materialize(std::move(self.base).get())), OperationTree>{
                    senluo::materialize(std::move(self.base).get())
                };
            }
        }

        template<class Self>
        constexpr auto principle(this Self&& self, custom_t = {})
        {
            using base_principle_t = decltype(senluo::principle(unwrap_opt_refer<Self>(self.base), custom_t{}));
            return operate_principle<base_principle_t, OperationTree>{
                senluo::principle(unwrap_opt_refer<Self>(self.base), custom_t{})
            };
        }
    };

    template<class T, auto OperationTree>
    struct tree_size<operate_view<T, OperationTree>>
    {
        static constexpr size_t value = size<T>;
    };

    namespace detail
    {
        template<auto OperationTree>
        struct operate_fn : adaptor_closure<operate_fn<OperationTree>>
        {
            template<class T>
            static constexpr decltype(auto) adapt(T&& t)
            {
                if constexpr(adaptor_closuroid<decltype(OperationTree)>)
                {
                    return FWD(t) || OperationTree;
                }
                else
                {
                    return operate_view<T, OperationTree>{ FWD(t) };
                }
            }

            template<array Indexes>
            static consteval auto dependencies()
            {
                return detail::operate_dependencies<Indexes, OperationTree>();
            }
        };
    }
}

// namespace senluo
// {
//     namespace detail
//     {
//         struct transform_fn : adaptor<transform_fn>
//         {
//             // Complex sfinae and noexcept are not currently provided.
//             template<class ArgTable, class Fn>
//             constexpr decltype(auto) adapt(ArgTable&& arg_table, Fn&& fn)const
//             {
//                 constexpr size_t n = size<ArgTable>;
//                 return zip(FWD(fn) | repeat<n>, FWD(arg_table)) 
//                     | operate<detail::replicate(operation_t::apply_invoke, shape<array<size_t, n>>)>;
//             }
//         };
//     }
    
//     inline namespace functors
//     {
//         inline constexpr detail::transform_fn transform{};
//     }

//     namespace detail
//     {
//         struct zip_transform_fn
//         {
//             // Complex sfinae and noexcept are not currently provided.
//             template<class Fn, class Args, class...Rest>
//             constexpr decltype(auto) operator()(Fn&& fn, Args&& args, Rest&&...rest)const
//             {
//                 constexpr size_t n = size<Args>;
//                 return zip(FWD(fn) | repeat<n>, FWD(args), FWD(rest)...) 
//                     | operate<detail::replicate(operation_t::apply_invoke, shape<array<size_t, n>>)>;
//             }
//         };
//     }

//     inline namespace functors
//     {
//         inline constexpr detail::zip_transform_fn zip_transform{};
//     }

//     namespace detail
//     {
//         struct mat_transform_fn : adaptor<mat_transform_fn>
//         {
//             template<class T, class Fn>
//             constexpr decltype(auto) adapt(T&& t, Fn&& fn) const
//             {
//                 using s = int[tensor_shape<T>[0]][tensor_shape<T>[1]];
//                 return mat_zip(FWD(fn) | repeat_as<s>, FWD(t)) 
//                     | operate<detail::replicate(operation_t::apply_invoke, shape<s>)>;
//             }
//         };
//     }

//     inline namespace functors
//     {
//         inline constexpr detail::mat_transform_fn mat_transform{};
//     }

//     namespace detail
//     {
//         struct mat_zip_transform_fn
//         {
//             template<class Fn, class Args, class...Rest>
//             constexpr decltype(auto) operator()(Fn&& fn, Args&& args, Rest&&...rest)const
//             {
//                 constexpr auto ts = tensor_shape<Args>;
//                 using s = array<array<int, ts[1]>, ts[0]>;
//                 return mat_zip(FWD(fn) | repeat_as<s>, FWD(args), FWD(rest)...) 
//                     | operate<detail::replicate(operation_t::apply_invoke, shape<s>)>;
//             }
//         };
//     }

//     inline namespace functors
//     {
//         inline constexpr detail::mat_zip_transform_fn mat_zip_transform{};
//     }

//     // namespace detail
//     // {
//     //     struct left_fold_fn : adaptor<left_fold_fn>
//     //     {
//     //         template<class Fn, class Args0, class Args1, class...Rest>
//     //         static constexpr decltype(auto) recursize_fn(const Fn& fn, const Args0& arg0, const Args1& arg1, const Rest&...rest)
//     //         {
//     //             if constexpr(sizeof...(rest) == 0)
//     //             {
//     //                 return fn(arg0, arg1);
//     //             }
//     //             else
//     //             {
//     //                 return recursize_fn(fn, fn(arg0, arg1), rest...);
//     //             }
//     //         }

//     //         template<class T, class Fn>
//     //         constexpr decltype(auto) adapt(const T& t, const Fn& fn) const
//     //         {
//     //             return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
//     //             {
//     //                 return recursize_fn(fn, tree_get<I>(t)...);
//     //             }(std::make_index_sequence<size<T> - 1uz>{});
//     //         }
//     //     };
//     // }

//     // inline namespace functors
//     // {
//     //     inline constexpr detail::left_fold_fn left_fold{};
//     // }
// }

#include "../tools/macro_undef.hpp"
#endif // SENLUO_OPERATE_HPP