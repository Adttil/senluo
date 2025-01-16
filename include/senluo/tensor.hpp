#ifndef SENLUO_TENSOR_HPP
#define SENLUO_TENSOR_HPP

#include "general.hpp"
#include "core.hpp"

#include "macro_define.hpp"

namespace senluo
{
    namespace detail
    {
        struct dot_t : adaptor<dot_t>
        {
            template<class L, class R>
            constexpr decltype(auto) adapt(L&& left, R&& right)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    auto&& left_seq = FWD(left) | refer | sequence;
                    auto&& right_seq = FWD(right) | refer | sequence;
                    return (... + ((FWD(left_seq) | subtree<I>) * (FWD(right_seq) | subtree<I>)));
                }(std::make_index_sequence<senluo::min(size<L>, size<R>)>{});
            }
        };
    }

    inline constexpr detail::dot_t dot{};

    namespace detail
    {
        struct mat_mul_vec_t : adaptor<mat_mul_vec_t>
        {
            template<class M, class V>
            constexpr decltype(auto) adapt(M&& mat, V&& vec)const
            {
                constexpr size_t row_count = size<M>;
                return zip_transform(dot, FWD(mat), FWD(vec) | repeat<row_count>);
            }
        };
    }

    inline constexpr detail::mat_mul_vec_t mat_mul_vec{};

    namespace detail
    {
        struct vec_mul_mat_t : adaptor<vec_mul_mat_t>
        {
            constexpr auto adapt(auto&& _vec, auto&& _mat)const
	        {
	            return mat_mul_vec(FWD(_mat) | transpose<>, FWD(_vec));
	        };
        };
    }

    inline constexpr detail::vec_mul_mat_t vec_mul_mat{};

    inline constexpr auto mat_mul = []<class L, class R>(L&& l, R&& r)
	{
        constexpr size_t n = size<L>;
		return zip_transform(vec_mul_mat, FWD(l), FWD(r) | repeat<n>);
	};
}

#include "macro_undef.hpp"
#endif