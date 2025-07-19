from itertools import permutations
component_name_groups = [['x', 'y', 'z', 'w'], ['r', 'g', 'b', 'a'], ['s', 't', 'p', 'q'], ['u', 'v']]

def generate_vec_accesser_1(component_names):
    result = ""
    n = len(component_names)
    for i in range(n):
        result += "constexpr decltype(auto) " + component_names[i] + "(this auto&& self) noexcept\n"
        result += "{\n"
        result += "    return senluo::tree_get<" + str(i) + "uz>(FWD(self, base));\n"
        result += "}\n\n"
    return result

def generate_vec_accesser_m(component_names, m):
    if(m == 1):
        return generate_vec_accesser_1(component_names)
    result = ""
    n = len(component_names)
    for indexes in permutations(range(n), m):
        result += "constexpr decltype(auto) "
        for index in indexes:
            result += component_names[index]
        result += "(this auto&& self) noexcept\n"
        result += "{\n"
        result += "    return detail::relayout_unchecked<tuple{ array{" + str(indexes[0]) + "uz}"
        for i in range(1, m):
            result += ", array{" + str(indexes[i]) + "uz}"
        result += " }>(FWD(self, base)) | as_vec;\n"
        result += "}\n\n"
    return result

def generate_vec_accesser(component_names):
    result = ""
    n = len(component_names)
    for m in range(1, n + 1):
        result += generate_vec_accesser_m(component_names, m)
    return result

with open("include/senluo/code_generate/vec_access.code", 'w') as file:
    for component_names in component_name_groups:
        file.write(generate_vec_accesser(component_names))