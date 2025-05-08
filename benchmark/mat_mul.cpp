#include <format>

#include <senluo/mat.hpp>
#include <senluo/tools/constant.hpp>

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
        tuple{ input[0], input[4], input[8], 0.0f },
        tuple{ input[1], input[5], input[9], 0.0f },
        tuple{ input[2], input[6], input[10], 0.0f },
        tuple{ input[3], input[7], input[11], 1.0f }
    };
    
    auto b = tuple{
        tuple{ input[12], input[16], input[20], 0.0f },
        tuple{ input[13], input[17], input[21], 0.0f },
        tuple{ input[14], input[18], input[22], 0.0f },
        tuple{ input[15], input[19], input[23], 1.0f }
    };

    for(size_t i = 0; i < n; ++i)
    {
        a = tuple
        {
            tuple{ 
                subtree<0, 0>(a) * subtree<0, 0>(b) + subtree<0, 1>(a) * subtree<1, 0>(b) + subtree<0, 2>(a) * subtree<2, 0>(b) + subtree<0, 3>(a) * subtree<3, 0>(b),
                subtree<1, 0>(a) * subtree<0, 0>(b) + subtree<1, 1>(a) * subtree<1, 0>(b) + subtree<1, 2>(a) * subtree<2, 0>(b) + subtree<1, 3>(a) * subtree<3, 0>(b),
                subtree<2, 0>(a) * subtree<0, 0>(b) + subtree<2, 1>(a) * subtree<1, 0>(b) + subtree<2, 2>(a) * subtree<2, 0>(b) + subtree<2, 3>(a) * subtree<3, 0>(b),
                subtree<3, 0>(a) * subtree<0, 0>(b) + subtree<3, 1>(a) * subtree<1, 0>(b) + subtree<3, 2>(a) * subtree<2, 0>(b) + subtree<3, 3>(a) * subtree<3, 0>(b)
            },
            tuple{ 
                subtree<0, 0>(a) * subtree<0, 1>(b) + subtree<0, 1>(a) * subtree<1, 1>(b) + subtree<0, 2>(a) * subtree<2, 1>(b) + subtree<0, 3>(a) * subtree<3, 1>(b),
                subtree<1, 0>(a) * subtree<0, 1>(b) + subtree<1, 1>(a) * subtree<1, 1>(b) + subtree<1, 2>(a) * subtree<2, 1>(b) + subtree<1, 3>(a) * subtree<3, 1>(b),
                subtree<2, 0>(a) * subtree<0, 1>(b) + subtree<2, 1>(a) * subtree<1, 1>(b) + subtree<2, 2>(a) * subtree<2, 1>(b) + subtree<2, 3>(a) * subtree<3, 1>(b),
                subtree<3, 0>(a) * subtree<0, 1>(b) + subtree<3, 1>(a) * subtree<1, 1>(b) + subtree<3, 2>(a) * subtree<2, 1>(b) + subtree<3, 3>(a) * subtree<3, 1>(b)
            },
            tuple{ 
                subtree<0, 0>(a) * subtree<0, 2>(b) + subtree<0, 1>(a) * subtree<1, 2>(b) + subtree<0, 2>(a) * subtree<2, 2>(b) + subtree<0, 3>(a) * subtree<3, 2>(b),
                subtree<1, 0>(a) * subtree<0, 2>(b) + subtree<1, 1>(a) * subtree<1, 2>(b) + subtree<1, 2>(a) * subtree<2, 2>(b) + subtree<1, 3>(a) * subtree<3, 2>(b),
                subtree<2, 0>(a) * subtree<0, 2>(b) + subtree<2, 1>(a) * subtree<1, 2>(b) + subtree<2, 2>(a) * subtree<2, 2>(b) + subtree<2, 3>(a) * subtree<3, 2>(b),
                subtree<3, 0>(a) * subtree<0, 2>(b) + subtree<3, 1>(a) * subtree<1, 2>(b) + subtree<3, 2>(a) * subtree<2, 2>(b) + subtree<3, 3>(a) * subtree<3, 2>(b)
            },
            tuple{ 
                subtree<0, 0>(a) * subtree<0, 3>(b) + subtree<0, 1>(a) * subtree<1, 3>(b) + subtree<0, 2>(a) * subtree<2, 3>(b) + subtree<0, 3>(a) * subtree<3, 3>(b),
                subtree<1, 0>(a) * subtree<0, 3>(b) + subtree<1, 1>(a) * subtree<1, 3>(b) + subtree<1, 2>(a) * subtree<2, 3>(b) + subtree<1, 3>(a) * subtree<3, 3>(b),
                subtree<2, 0>(a) * subtree<0, 3>(b) + subtree<2, 1>(a) * subtree<1, 3>(b) + subtree<2, 2>(a) * subtree<2, 3>(b) + subtree<2, 3>(a) * subtree<3, 3>(b),
                subtree<3, 0>(a) * subtree<0, 3>(b) + subtree<3, 1>(a) * subtree<1, 3>(b) + subtree<3, 2>(a) * subtree<2, 3>(b) + subtree<3, 3>(a) * subtree<3, 3>(b)
            }
        };
    }

    return a;
}

result_t senluo_ver(size_t n, const array<float, 24>& input)
{
    auto a = mat{tuple{
        tuple{ input[0], input[4], input[8], 0.0f },
        tuple{ input[1], input[5], input[9], 0.0f },
        tuple{ input[2], input[6], input[10], 0.0f },
        tuple{ input[3], input[7], input[11], 1.0f }
    }};
    
    auto b = mat{tuple{
        tuple{ input[12], input[16], input[20], 0.0f },
        tuple{ input[13], input[17], input[21], 0.0f },
        tuple{ input[14], input[18], input[22], 0.0f },
        tuple{ input[15], input[19], input[23], 1.0f }
    }};

    for(size_t i = 0; i < n; ++i)
    {
        a = a * b;
    }

    return a.raw_base();
}

result_t heterogeneous(size_t n, const array<float, 24>& input)
{
    auto a = mat{tuple{
        tuple{ input[0], input[4], input[8], constant_t<0>{} },
        tuple{ input[1], input[5], input[9], constant_t<0>{} },
        tuple{ input[2], input[6], input[10], constant_t<0>{} },
        tuple{ input[3], input[7], input[11], constant_t<1>{} }
    }};
    
    auto b = mat{tuple{
        tuple{ input[12], input[16], input[20], constant_t<0>{} },
        tuple{ input[13], input[17], input[21], constant_t<0>{} },
        tuple{ input[14], input[18], input[22], constant_t<0>{} },
        tuple{ input[15], input[19], input[23], constant_t<1>{} }
    }};

    for(size_t i = 0; i < n; ++i)
    {
        a = a * b;
    }

    return a.raw_base() | make<result_t>;
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