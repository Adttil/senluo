#include <iostream>
#include <chrono>
#include <fstream>
#include <ranges>
#include <span>
#include "senluo/array.hpp"

namespace senluo 
{
    template<class F>
    struct test_unit
    {
        const char* name;
        F task;
    };

    template<class F>
    test_unit(const char*, F) -> test_unit<F>;

#define TEST_UNIT(fn) test_unit{ #fn, fn }

#define BENCHMARK_PATH "../../../benchmark/"

    template<size_t N, class F, class...Args>
    void benchmark(array<test_unit<F>, N> tests, const char* test_seq_file_path, const Args&...args)
    {
        using result_t = std::invoke_result_t<const F&, const Args&...>;
        struct benchresult
        {
            result_t result;
            std::chrono::system_clock::duration time_dur;
        };
        array<benchresult, N> results;

        {
            std::ifstream test_seq_file(test_seq_file_path);
            for(size_t i = 0; i < N; ++i)
            {
                size_t s;
                test_seq_file >> s;
                auto start = std::chrono::system_clock::now();
                results[s].result = tests[s].task(args...);
                results[s].time_dur = std::chrono::system_clock::now() - start;
            }
        }

        for(size_t i = 1; i < N; ++i)
        {
            if(results[i].result != results[0].result)
            {
                std::puts("error: Inconsistent test results!");
                return;
            }
        }

        for(const auto& [test, result] : std::views::zip(tests, results))
        {
            std::cout << test.name << "\n\t" << std::chrono::duration_cast<std::chrono::milliseconds>(result.time_dur) << '\n';
        }
    }
}