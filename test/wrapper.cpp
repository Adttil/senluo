//#include <senluo/core.hpp>
#include <senluo/tools/wrapper.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(wrapper, ideal_unwrap_t)
{
    //T
    MAGIC_TCHECK(ideal_unwrap_t<int>, int);
    MAGIC_TCHECK(ideal_unwrap_t<const int>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<int&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<const int&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<int&&>, int);
    MAGIC_TCHECK(ideal_unwrap_t<const int&&>, const int);

    //wrapper<T>
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int>>, int);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int>>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int&>>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int&>>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int&&>>, int&&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int&&>>, const int&&);

    //const wrapper<T>
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int>>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int>>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int&>>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int&>>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int&&>>, int&&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int&&>>, const int&&);

    //wrapper<T>&
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int>&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int>&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int&>&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int&>&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int&&>&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int&&>&>, const int&);

    //const wrapper<T>&
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int>&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int>&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int&>&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int&>&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int&&>&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int&&>&>, const int&);

    //wrapper<T>&&
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int>&&>, int);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int>&&>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int&>&&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int&>&&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<int&&>&&>, int&&);
    MAGIC_TCHECK(ideal_unwrap_t<wrapper<const int&&>&&>, const int&&);

    //const wrapper<T>&&
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int>&&>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int>&&>, const int);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int&>&&>, int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int&>&&>, const int&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<int&&>&&>, int&&);
    MAGIC_TCHECK(ideal_unwrap_t<const wrapper<const int&&>&&>, const int&&);
}

TEST(wrapper, ideal_store_t)
{
    //T
    MAGIC_TCHECK(ideal_store_t<int>, int);
    MAGIC_TCHECK(ideal_store_t<const int>, int);
    MAGIC_TCHECK(ideal_store_t<int&>, int&);
    MAGIC_TCHECK(ideal_store_t<const int&>, const int&);
    MAGIC_TCHECK(ideal_store_t<int&&>, int);
    MAGIC_TCHECK(ideal_store_t<const int&&>, int);

    //wrapper<T>
    MAGIC_TCHECK(ideal_store_t<wrapper<int>>, int);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int>>, int);
    MAGIC_TCHECK(ideal_store_t<wrapper<int&>>, int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int&>>, const int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<int&&>>, int&&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int&&>>, const int&&);

    //const wrapper<T>
    MAGIC_TCHECK(ideal_store_t<const wrapper<int>>, int);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int>>, int);
    MAGIC_TCHECK(ideal_store_t<const wrapper<int&>>, int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int&>>, const int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<int&&>>, int&&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int&&>>, const int&&);

    //wrapper<T>&
    MAGIC_TCHECK(ideal_store_t<wrapper<int>&>, int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int>&>, const int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<int&>&>, int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int&>&>, const int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<int&&>&>, int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int&&>&>, const int&);

    //const wrapper<T>&
    MAGIC_TCHECK(ideal_store_t<const wrapper<int>&>, const int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int>&>, const int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<int&>&>, int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int&>&>, const int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<int&&>&>, int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int&&>&>, const int&);

    //wrapper<T>&&
    MAGIC_TCHECK(ideal_store_t<wrapper<int>&&>, int);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int>&&>, int);
    MAGIC_TCHECK(ideal_store_t<wrapper<int&>&&>, int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int&>&&>, const int&);
    MAGIC_TCHECK(ideal_store_t<wrapper<int&&>&&>, int&&);
    MAGIC_TCHECK(ideal_store_t<wrapper<const int&&>&&>, const int&&);

    //const wrapper<T>&&
    MAGIC_TCHECK(ideal_store_t<const wrapper<int>&&>, int);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int>&&>, int);
    MAGIC_TCHECK(ideal_store_t<const wrapper<int&>&&>, int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int&>&&>, const int&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<int&&>&&>, int&&);
    MAGIC_TCHECK(ideal_store_t<const wrapper<const int&&>&&>, const int&&);
}

TEST(wrapper, get)
{
    int i = 233;
    
    wrapper<int> t{ i };
    MAGIC_CHECK((t.get()), i, int&);
    MAGIC_CHECK((std::as_const(t).get()), i, const int&);
    MAGIC_CHECK((std::move(t).get()), i, int&&);
    MAGIC_CHECK((std::move(std::as_const(t)).get()), i, const int&&);

    wrapper<const int> ct{ i };
    MAGIC_CHECK((ct.get()), i, const int&);
    MAGIC_CHECK((std::as_const(ct).get()), i, const int&);
    MAGIC_CHECK((std::move(ct).get()), i, const int&&);
    MAGIC_CHECK((std::move(std::as_const(ct)).get()), i, const int&&);

    
    wrapper<int&> r{ i };
    MAGIC_CHECK((r.get()), i, int&);
    MAGIC_CHECK((std::as_const(r).get()), i, int&);
    MAGIC_CHECK((std::move(r).get()), i, int&);
    MAGIC_CHECK((std::move(std::as_const(r)).get()), i, int&);

    wrapper<const int&> cr{ i };
    MAGIC_CHECK((cr.get()), i, const int&);
    MAGIC_CHECK((std::as_const(cr).get()), i, const int&);
    MAGIC_CHECK((std::move(cr).get()), i, const int&);
    MAGIC_CHECK((std::move(std::as_const(cr)).get()), i, const int&);

    wrapper<int&&> rr{ std::move(i) };
    MAGIC_CHECK((rr.get()), i, int&);
    MAGIC_CHECK((std::as_const(rr).get()), i, int&);
    MAGIC_CHECK((std::move(rr).get()), i, int&&);
    MAGIC_CHECK((std::move(std::as_const(rr)).get()), i, int&&);

    wrapper<const int&&> crr{ std::move(i) };
    MAGIC_CHECK((crr.get()), i, const int&);
    MAGIC_CHECK((std::as_const(crr).get()), i, const int&);
    MAGIC_CHECK((std::move(crr).get()), i, const int&&);
    MAGIC_CHECK((std::move(std::as_const(crr)).get()), i, const int&&);
}

TEST(wrapper, value)
{
    int i = 233;
    
    wrapper<int> t{ i };
    MAGIC_CHECK((t.value()), i, int&);
    MAGIC_CHECK((std::as_const(t).value()), i, const int&);
    MAGIC_CHECK((std::move(t).value()), i, int);
    MAGIC_CHECK((std::move(std::as_const(t)).value()), i, int);

    wrapper<const int> ct{ i };
    MAGIC_CHECK((ct.value()), i, const int&);
    MAGIC_CHECK((std::as_const(ct).value()), i, const int&);
    MAGIC_CHECK((std::move(ct).value()), i, int);
    MAGIC_CHECK((std::move(std::as_const(ct)).value()), i, int);

    
    wrapper<int&> r{ i };
    MAGIC_CHECK((r.value()), i, int&);
    MAGIC_CHECK((std::as_const(r).value()), i, int&);
    MAGIC_CHECK((std::move(r).value()), i, int&);
    MAGIC_CHECK((std::move(std::as_const(r)).value()), i, int&);

    wrapper<const int&> cr{ i };
    MAGIC_CHECK((cr.value()), i, const int&);
    MAGIC_CHECK((std::as_const(cr).value()), i, const int&);
    MAGIC_CHECK((std::move(cr).value()), i, const int&);
    MAGIC_CHECK((std::move(std::as_const(cr)).value()), i, const int&);

    wrapper<int&&> rr{ std::move(i) };
    MAGIC_CHECK((rr.value()), i, int&);
    MAGIC_CHECK((std::as_const(rr).value()), i, int&);
    MAGIC_CHECK((std::move(rr).value()), i, int&&);
    MAGIC_CHECK((std::move(std::as_const(rr)).value()), i, int&&);

    wrapper<const int&&> crr{ std::move(i) };
    MAGIC_CHECK((crr.value()), i, const int&);
    MAGIC_CHECK((std::as_const(crr).value()), i, const int&);
    MAGIC_CHECK((std::move(crr).value()), i, const int&&);
    MAGIC_CHECK((std::move(std::as_const(crr)).value()), i, const int&&);
}