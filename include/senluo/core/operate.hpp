#ifndef SENLUO_OPERATE_HPP
#define SENLUO_OPERATE_HPP

#include "../tools/adaptor.hpp"
#include "../tools/constant.hpp"
#include "tree.hpp"
#include "relayout.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    namespace detail 
    {
        struct apply_t : adaptor<apply_t>
        {
            // Complex sfinae and noexcept are not currently provided.
            template<class Args, class Fn>
            constexpr decltype(auto) adapt(Args&& args, Fn&& fn)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    return FWD(fn)(tree_get<I>(FWD(args))...);
                }(std::make_index_sequence<size<Args>>{});
            }
        };
        
        struct apply_invoke_t : adaptor_closure<apply_invoke_t>
        {
            // Complex sfinae and noexcept are not currently provided.
            template<class T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    decltype(auto) sep_tree = seperate(FWD(tree));
                    return tree_get<0uz>(FWD(sep_tree))(tree_get<I + 1uz>(FWD(sep_tree))...);
                }(std::make_index_sequence<size<T> - 1uz>{});
            }
        };
    }

    inline constexpr detail::apply_t apply{};

    inline constexpr detail::apply_invoke_t apply_invoke{};
}

namespace senluo::detail
{
    template<size_t I, class OperationTree>
    constexpr auto operation_tree_get(const OperationTree& operation_tree)
    {
        if constexpr(std::same_as<OperationTree, operation_t>)
        {
            return operation_tree;
        }
        else
        {
            return get<I>(operation_tree);
        }
    }
}

namespace senluo
{
    template<class T, auto OperationTree>
    struct operate_tree : based_on<T>
    {
        template<size_t I, unwarp_derived_from<operate_tree> Self> 
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            constexpr auto op_subtree = detail::operation_tree_get<I>(OperationTree);
            if constexpr(not std::same_as<decltype(op_subtree), const operation_t>)
            {
                return operate_tree<decltype(tree_get<I>(FWD(self).unwrap_base())), op_subtree>
                {
                    tree_get<I>(FWD(self).unwrap_base())
                };
            }
            else if constexpr(op_subtree == operation_t::none)
            {
                return tree_get<I>(FWD(self).unwrap_base());
            }
            else
            {
                return apply_invoke(tree_get<I>(FWD(self).unwrap_base()));
            }
        }
    };

    template<class T, auto OperationTree>
    struct tree_size<operate_tree<T, OperationTree>>
    {
        static constexpr size_t value = size<T>;
    };

    namespace detail
    {
        template<auto Indexes, auto OperationTree>
        constexpr auto operate_used_index()
        {
            if constexpr(Indexes.size() == 0uz)
            {
                return indexes_of_whole;
            }
            else if constexpr(std::same_as<decltype(OperationTree), operation_t>)
            {
                if constexpr(OperationTree == operation_t::apply_invoke)
                {
                    return indexes_of_whole;
                }
                else
                {
                    return Indexes;
                }
            }
            else
            {
                return detail::array_cat(
                    array{Indexes[0]}, 
                    detail::operate_used_index<detail::array_drop<1uz>(Indexes), get<Indexes[0]>(OperationTree)>()
                ); 
            }
        }

        template<auto OperationTree, class T>
        constexpr decltype(auto) operate_unchecked(T &&t)
        {
            if constexpr(not std::same_as<decltype(OperationTree), operation_t>)
            {
                return operate_tree<senluo::unwrap_t<T>, OperationTree>{ unwrap_fwd(FWD(t)) };
            }
            else if constexpr(OperationTree == operation_t::none)
            {
                return pass(FWD(t));
            }
            else
            {
                return apply_invoke(FWD(t));
            }
        }

        template<auto OperationTree>
        struct operate_fn : adaptor_closure<operate_fn<OperationTree>>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                return detail::operate_unchecked<OperationTree>(FWD(t));
            }

            template<class S>
            static constexpr operate_fn<detail::fold_operation_tree<detail::replicate<S>(OperationTree)>()> replicate(S = {}) noexcept
            {
                return {};
            }

            template<auto Indexes>
            friend constexpr auto operator/(constant_t<Indexes>, operate_fn)
            {
                return []<size_t...I>(std::index_sequence<I...>)
                {
                    return constant_t<tuple{ detail::operate_used_index<get<I>(Indexes), OperationTree>()... }>{};
                }(std::make_index_sequence<std::tuple_size_v<decltype(Indexes)>>{});
            }
        };
    }

    inline namespace functors
    {
        template<auto OperationTree>
        inline constexpr detail::operate_fn<detail::fold_operation_tree<OperationTree>()> operate{};
    }
}

namespace senluo
{
    namespace detail
    {
        struct transform_fn : adaptor<transform_fn>
        {
            // Complex sfinae and noexcept are not currently provided.
            template<class ArgTable, class Fn>
            constexpr decltype(auto) adapt(ArgTable&& arg_table, Fn&& fn)const
            {
                constexpr size_t n = size<ArgTable>;
                return zip(FWD(fn) | repeat<n>, FWD(arg_table)) 
                    | operate<detail::replicate(operation_t::apply_invoke, shape<array<size_t, n>>)>;
            }
        };
    }
    
    inline namespace functors
    {
        inline constexpr detail::transform_fn transform{};
    }

    namespace detail
    {
        struct zip_transform_fn
        {
            // Complex sfinae and noexcept are not currently provided.
            template<class Fn, class Args, class...Rest>
            constexpr decltype(auto) operator()(Fn&& fn, Args&& args, Rest&&...rest)const
            {
                constexpr size_t n = size<Args>;
                return zip(FWD(fn) | repeat<n>, FWD(args), FWD(rest)...) 
                    | operate<detail::replicate(operation_t::apply_invoke, shape<array<size_t, n>>)>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::zip_transform_fn zip_transform{};
    }

    namespace detail
    {
        struct mat_transform_fn : adaptor<mat_transform_fn>
        {
            template<class T, class Fn>
            constexpr decltype(auto) adapt(T&& t, Fn&& fn) const
            {
                using s = int[tensor_shape<T>[0]][tensor_shape<T>[1]];
                return mat_zip(FWD(fn) | repeat_as<s>, FWD(t)) 
                    | operate<detail::replicate(operation_t::apply_invoke, shape<s>)>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::mat_transform_fn mat_transform{};
    }

    namespace detail
    {
        struct mat_zip_transform_fn
        {
            template<class Fn, class Args, class...Rest>
            constexpr decltype(auto) operator()(Fn&& fn, Args&& args, Rest&&...rest)const
            {
                constexpr auto ts = tensor_shape<Args>;
                using s = array<array<int, ts[1]>, ts[0]>;
                return mat_zip(FWD(fn) | repeat_as<s>, FWD(args), FWD(rest)...) 
                    | operate<detail::replicate(operation_t::apply_invoke, shape<s>)>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::mat_zip_transform_fn mat_zip_transform{};
    }

    // namespace detail
    // {
    //     struct left_fold_fn : adaptor<left_fold_fn>
    //     {
    //         template<class Fn, class Args0, class Args1, class...Rest>
    //         static constexpr decltype(auto) recursize_fn(const Fn& fn, const Args0& arg0, const Args1& arg1, const Rest&...rest)
    //         {
    //             if constexpr(sizeof...(rest) == 0)
    //             {
    //                 return fn(arg0, arg1);
    //             }
    //             else
    //             {
    //                 return recursize_fn(fn, fn(arg0, arg1), rest...);
    //             }
    //         }

    //         template<class T, class Fn>
    //         constexpr decltype(auto) adapt(const T& t, const Fn& fn) const
    //         {
    //             return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
    //             {
    //                 return recursize_fn(fn, tree_get<I>(t)...);
    //             }(std::make_index_sequence<size<T> - 1uz>{});
    //         }
    //     };
    // }

    // inline namespace functors
    // {
    //     inline constexpr detail::left_fold_fn left_fold{};
    // }
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_OPERATE_HPP