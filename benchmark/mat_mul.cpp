#include <format>

#include <senluo/tensor.hpp>
#include <senluo/constant.hpp>

#include "benchmark_tool.hpp"

using namespace senluo;

using result_t = tuple<
    tuple<float, float, float, float>,
    tuple<float, float, float, float>,
    tuple<float, float, float, float>,
    tuple<float, float, float, float>
>;

result_t flat(size_t n, const array<float, 24>& input)
{
    auto a = tuple{
        tuple{ input[0], input[1], input[2], input[3] },
        tuple{ input[4], input[5], input[6], input[7] },
        tuple{ input[8], input[9], input[10], input[11] },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    
    auto b = tuple{
        tuple{ input[12], input[13], input[14], input[15] },
        tuple{ input[16], input[17], input[18], input[19] },
        tuple{ input[20], input[21], input[22], input[23] },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    for(size_t i = 0; i < n; ++i)
    {
        a = tuple
        {
            tuple{ 
                subtree<0, 0>(a) * subtree<0, 0>(b) + subtree<0, 1>(a) * subtree<1, 0>(b) + subtree<0, 2>(a) * subtree<2, 0>(b) + subtree<0, 3>(a) * subtree<3, 0>(b),
                subtree<0, 0>(a) * subtree<0, 1>(b) + subtree<0, 1>(a) * subtree<1, 1>(b) + subtree<0, 2>(a) * subtree<2, 1>(b) + subtree<0, 3>(a) * subtree<3, 1>(b),
                subtree<0, 0>(a) * subtree<0, 2>(b) + subtree<0, 1>(a) * subtree<1, 2>(b) + subtree<0, 2>(a) * subtree<2, 2>(b) + subtree<0, 3>(a) * subtree<3, 2>(b),
                subtree<0, 0>(a) * subtree<0, 3>(b) + subtree<0, 1>(a) * subtree<1, 3>(b) + subtree<0, 2>(a) * subtree<2, 3>(b) + subtree<0, 3>(a) * subtree<3, 3>(b)
            },
            tuple{ 
                subtree<1, 0>(a) * subtree<0, 0>(b) + subtree<1, 1>(a) * subtree<1, 0>(b) + subtree<1, 2>(a) * subtree<2, 0>(b) + subtree<1, 3>(a) * subtree<3, 0>(b),
                subtree<1, 0>(a) * subtree<0, 1>(b) + subtree<1, 1>(a) * subtree<1, 1>(b) + subtree<1, 2>(a) * subtree<2, 1>(b) + subtree<1, 3>(a) * subtree<3, 1>(b),
                subtree<1, 0>(a) * subtree<0, 2>(b) + subtree<1, 1>(a) * subtree<1, 2>(b) + subtree<1, 2>(a) * subtree<2, 2>(b) + subtree<1, 3>(a) * subtree<3, 2>(b),
                subtree<1, 0>(a) * subtree<0, 3>(b) + subtree<1, 1>(a) * subtree<1, 3>(b) + subtree<1, 2>(a) * subtree<2, 3>(b) + subtree<1, 3>(a) * subtree<3, 3>(b)
            },
            tuple{ 
                subtree<2, 0>(a) * subtree<0, 0>(b) + subtree<2, 1>(a) * subtree<1, 0>(b) + subtree<2, 2>(a) * subtree<2, 0>(b) + subtree<2, 3>(a) * subtree<3, 0>(b),
                subtree<2, 0>(a) * subtree<0, 1>(b) + subtree<2, 1>(a) * subtree<1, 1>(b) + subtree<2, 2>(a) * subtree<2, 1>(b) + subtree<2, 3>(a) * subtree<3, 1>(b),
                subtree<2, 0>(a) * subtree<0, 2>(b) + subtree<2, 1>(a) * subtree<1, 2>(b) + subtree<2, 2>(a) * subtree<2, 2>(b) + subtree<2, 3>(a) * subtree<3, 2>(b),
                subtree<2, 0>(a) * subtree<0, 3>(b) + subtree<2, 1>(a) * subtree<1, 3>(b) + subtree<2, 2>(a) * subtree<2, 3>(b) + subtree<2, 3>(a) * subtree<3, 3>(b)
            },
            tuple{ 
                subtree<3, 0>(a) * subtree<0, 0>(b) + subtree<3, 1>(a) * subtree<1, 0>(b) + subtree<3, 2>(a) * subtree<2, 0>(b) + subtree<3, 3>(a) * subtree<3, 0>(b),
                subtree<3, 0>(a) * subtree<0, 1>(b) + subtree<3, 1>(a) * subtree<1, 1>(b) + subtree<3, 2>(a) * subtree<2, 1>(b) + subtree<3, 3>(a) * subtree<3, 1>(b),
                subtree<3, 0>(a) * subtree<0, 2>(b) + subtree<3, 1>(a) * subtree<1, 2>(b) + subtree<3, 2>(a) * subtree<2, 2>(b) + subtree<3, 3>(a) * subtree<3, 2>(b),
                subtree<3, 0>(a) * subtree<0, 3>(b) + subtree<3, 1>(a) * subtree<1, 3>(b) + subtree<3, 2>(a) * subtree<2, 3>(b) + subtree<3, 3>(a) * subtree<3, 3>(b)
            }
        };
    }

    return a;
}

result_t senluo_ver(size_t n, const array<float, 24>& input)
{
    auto a = tuple{
        tuple{ input[0], input[1], input[2], input[3] },
        tuple{ input[4], input[5], input[6], input[7] },
        tuple{ input[8], input[9], input[10], input[11] },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    
    auto b = tuple{
        tuple{ input[12], input[13], input[14], input[15] },
        tuple{ input[16], input[17], input[18], input[19] },
        tuple{ input[20], input[21], input[22], input[23] },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    for(size_t i = 0; i < n; ++i)
    {
        a = +mat_mul(a, b);
    }

    return a;
}

result_t heterogeneous(size_t n, const array<float, 24>& input)
{
    auto a = tuple{
        tuple{ input[0], input[1], input[2], input[3] },
        tuple{ input[4], input[5], input[6], input[7] },
        tuple{ input[8], input[9], input[10], input[11] },
        std::tuple{ constant_t<0>{}, constant_t<0>{}, constant_t<0>{}, constant_t<1>{} }
    };
    
    auto b = tuple{
        tuple{ input[12], input[13], input[14], input[15] },
        tuple{ input[16], input[17], input[18], input[19] },
        tuple{ input[20], input[21], input[22], input[23] },
        std::tuple{ constant_t<0>{}, constant_t<0>{}, constant_t<0>{}, constant_t<1>{} }
    };

    for(size_t i = 0; i < n; ++i)
    {
        a = +mat_mul(a, b);
    }

    return tuple
    {
        subtree<0>(a),
        subtree<1>(a),
        subtree<2>(a),
        tuple{ (float)subtree<3, 0>(a), (float)subtree<3, 1>(a), (float)subtree<3, 2>(a), (float)subtree<3, 3>(a) }
    };
}

int main()
{
    constexpr auto tests = array{ TEST_UNIT(flat), TEST_UNIT(senluo_ver), TEST_UNIT(heterogeneous) };
    size_t n;
    auto input = array<float, 24>{};
    float init, zero, multiplier;
    {
        std::ifstream test_input(BENCHMARK_PATH "mat_mul_input.txt");
        test_input >> n;
        for(size_t i = 0; i < 24; ++i) test_input >> input[i];
    }
    benchmark(tests, BENCHMARK_PATH "mat_mul_test_seq.txt", n, input);
}