#ifndef SENLUO_ARRAY_HPP
#define SENLUO_ARRAY_HPP

#include <cstddef>
#include <tuple>

#if __STDC_HOSTED__ && !defined(SENLUO_DONOT_USE_STD_ARRAY)

#include <array>

namespace senluo
{
    using std::array;
}

#else// For freestanding implementation which maybe do not has std::array.

#include "general.hpp"

#include "macro_define.hpp"

namespace senluo
{
    // This simple array is only for this library. 
    // It is not guaranteed to have exactly the same behavior as std::array.
    template<class T, size_t N>
    struct array
    {
        using value_type = T;

        T data[N];

        template<class Self>
        constexpr auto&& operator[](this Self&& self, size_t i)noexcept
        {
            return (self.data[i]);
        }

        static constexpr size_t size()noexcept
        {
            return N;
        }

        constexpr auto begin(this auto&& self)noexcept
        {
            return self.data;
        }

        constexpr auto end(this auto&& self)noexcept
        {
            return self.data + N;
        }

        template<size_t i, derived_from<array> Self>
        friend constexpr decltype(auto) get(Self&& self)noexcept
        {
            return static_cast<fwd_type<decltype(self.data[i]), Self>>(self.data[i]);
        }

        friend constexpr bool operator==(const array&, const array&) = default;
    };

    template<class T>
    struct array<T, 0uz>
    {
        using value_type = T;

        static constexpr size_t size()
        {
            return 0uz;
        }

        constexpr T& operator[](size_t i) noexcept;
        constexpr const T& operator[](size_t i)const noexcept;

        constexpr T* begin()noexcept
        {
            return nullptr;
        }

        constexpr const T* begin()const noexcept
        {
            return nullptr;
        }

        constexpr T* end()noexcept
        {
            return nullptr;
        }

        constexpr const T* end()const noexcept
        {
            return nullptr;
        }

        friend constexpr bool operator==(const array&, const array&) = default;
    };

    template<class...T>
    array(T...) -> array<std::common_type_t<T...>, sizeof...(T)>;
}

template<class T, size_t N>
struct std::tuple_size<senluo::array<T, N>> : std::integral_constant<size_t, N>{};

template<size_t I, class T, size_t N>
struct std::tuple_element<I, senluo::array<T, N>>{
    using type = T;
};

#endif

namespace senluo::detail
{
    template<size_t N, class T, size_t M>
    constexpr array<T, N> array_take(const array<T, M>& arr)
    {
        array<T, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = arr[i];
        }
        return result;
    }

    template<size_t N, class T, size_t M>
    constexpr array<T, M - N> array_drop(const array<T, M>& arr)
    {
        array<T, M - N> result;
        for(size_t i = 0; i < M - N; ++i)
        {
            result[i] = arr[i + N];
        }
        return result;
    }

    template<class T, size_t N, size_t M>
    constexpr array<T, N + M> two_array_cat(const array<T, N>& arr1, const array<T, M>& arr2)
    {
        array<T, N + M> result;

        for (size_t i = 0; i < N; ++i)
        {
            result[i] = arr1[i];
        }
        for (size_t i = 0; i < M; ++i)
        {
            result[N + i] = arr2[i];
        }

        return result;
    }
    
    template<class T, size_t N,size_t...Rest>
    constexpr array<T, (N + ... + Rest)> array_cat(const array<T, N>& arr, const array<T, Rest>&...rest)
    {
        if constexpr (sizeof...(rest) == 0)
        {
            return arr;
        }
        else
        {
            return detail::two_array_cat(arr, detail::array_cat(rest...));
        }
    }
}

#include "macro_undef.hpp"
#endif