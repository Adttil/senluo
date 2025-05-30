#include <senluo/mat.hpp>

using namespace senluo;

int main()
{
    auto mat1 = array{ array{1,3}, array{2,4} } | as_mat;
    auto mat2 = array{ array{5,7}, array{6,8} } | as_mat;

    auto mat5 = mat1 * mat2;

    // return mat5.r<0>().x() + mat5.r<0>().y() + 
    //        mat5.r<1>().x() + mat5.r<1>().y();
}