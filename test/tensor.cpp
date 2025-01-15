#include <senluo/tensor.hpp>
#include <senluo/make.hpp>
#include <random>
#include "test_tool.hpp"

using namespace senluo;

auto rnd = std::default_random_engine{ 0 };
auto dis = std::uniform_real_distribution<float>{ 0.0f, 1.0f };
size_t N;
int s;

TEST(tensor, mat_mul)
{
    std::cout << "input N: "; std::cin >> N; 
    std::cout << "input seed: "; std::cin >> s; 


    static constexpr auto a = tuple{
        tuple{ 1, 2 },
        tuple{ 3, 4 }
    };

    constexpr auto b = mat_mul(a, a) | make<array<array<int, 2>, 2>>;

    MAGIC_CHECK(7, b | subtree<0, 0>);
    MAGIC_CHECK(10, b | subtree<0, 1>);
    MAGIC_CHECK(15, b | subtree<1, 0>);
    MAGIC_CHECK(22, b | subtree<1, 1>);
}
TEST(tensor, mat_mul_flat)
{
    rnd.seed(s);
    auto a = tuple{
        tuple{ 1.0f, 0.0f, 0.0f, 0.0f },
        tuple{ 0.0f, 1.0f, 0.0f, 0.0f },
        tuple{ 0.0f, 0.0f, 1.0f, 0.0f },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    
    auto b = tuple{
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    for(size_t i = 0; i < N; ++i)
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

    std::cout << std::format("{},{},{},{}\n{},{},{},{}\n{},{},{},{}\n,{},{},{},{}",
                 subtree<0, 0>(a), subtree<0, 1>(a), subtree<0, 2>(a), subtree<0, 3>(a),
                 subtree<1, 0>(a), subtree<1, 1>(a), subtree<1, 2>(a), subtree<1, 3>(a),
                 subtree<2, 0>(a), subtree<2, 1>(a), subtree<2, 2>(a), subtree<2, 3>(a),
                 subtree<3, 0>(a), subtree<3, 1>(a), subtree<3, 2>(a), subtree<3, 3>(a)
                 );
    //std::cout << subtree<0, 0>(a) << ", " << subtree<0, 1>(a) << ", " << subtree<1, 0>(a) << ", " << subtree<1, 1>(a) << '\n';
}
TEST(tensor, mat_mul_rzx)
{
    rnd.seed(s);
    auto a = tuple{
        tuple{ 1.0f, 0.0f, 0.0f, 0.0f },
        tuple{ 0.0f, 1.0f, 0.0f, 0.0f },
        tuple{ 0.0f, 0.0f, 1.0f, 0.0f },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };
    
    auto b = tuple{
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ 0.0f, 0.0f, 0.0f, 1.0f }
    };

    for(size_t i = 0; i < N; ++i)
    {
        a = mat_mul(a, b) | make<decltype(a)>;
    }

    std::cout << std::format("{},{},{},{}\n{},{},{},{}\n{},{},{},{}\n,{},{},{},{}",
                 subtree<0, 0>(a), subtree<0, 1>(a), subtree<0, 2>(a), subtree<0, 3>(a),
                 subtree<1, 0>(a), subtree<1, 1>(a), subtree<1, 2>(a), subtree<1, 3>(a),
                 subtree<2, 0>(a), subtree<2, 1>(a), subtree<2, 2>(a), subtree<2, 3>(a),
                 subtree<3, 0>(a), subtree<3, 1>(a), subtree<3, 2>(a), subtree<3, 3>(a)
                 );
    //std::cout << subtree<0, 0>(a) << ", " << subtree<0, 1>(a) << ", " << subtree<1, 0>(a) << ", " << subtree<1, 1>(a) << '\n';
}

TEST(tensor, mat_mul_rzx_spec)
{
    rnd.seed(s);
    auto a = tuple{
        tuple{ 1.0f, 0.0f, 0.0f, 0.0f },
        tuple{ 0.0f, 1.0f, 0.0f, 0.0f },
        tuple{ 0.0f, 0.0f, 1.0f, 0.0f },
        std::tuple{ constant_t<0>{}, constant_t<0>{}, constant_t<0>{}, constant_t<1>{} }
    };
    
    auto b = tuple{
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        tuple{ dis(rnd), dis(rnd), dis(rnd), dis(rnd) },
        std::tuple{ constant_t<0>{}, constant_t<0>{}, constant_t<0>{}, constant_t<1>{} }
    };

    for(size_t i = 0; i < N; ++i)
    {
        a = mat_mul(a, b) | make<decltype(a)>;
    }

    std::cout << std::format("{},{},{},{}\n{},{},{},{}\n{},{},{},{}\n,{},{},{},{}",
                 subtree<0, 0>(a), subtree<0, 1>(a), subtree<0, 2>(a), subtree<0, 3>(a),
                 subtree<1, 0>(a), subtree<1, 1>(a), subtree<1, 2>(a), subtree<1, 3>(a),
                 subtree<2, 0>(a), subtree<2, 1>(a), subtree<2, 2>(a), subtree<2, 3>(a),
                 subtree<3, 0>(a).value, subtree<3, 1>(a).value, subtree<3, 2>(a).value, subtree<3, 3>(a).value
                 );
    //std::cout << subtree<0, 0>(a) << ", " << subtree<0, 1>(a) << ", " << subtree<1, 0>(a) << ", " << subtree<1, 1>(a) << '\n';
}

TEST(tensor, triple_mat_mul)
{
    static constexpr auto a = tuple{
        tuple{ 1, 2 },
        tuple{ 3, 4 }
    };

    constexpr auto b = mat_mul(a, mat_mul(a, a)) | make<array<array<int, 2>, 2>>;

    std::cout << std::format("{},{}\n{},{}\n", subtree<0, 0>(b), subtree<0, 1>(b),  subtree<1, 0>(b), subtree<1, 1>(b));
}