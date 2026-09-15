#pragma once
#include <string>

namespace solver {

struct Input {
    std::string name;
};

struct Output {
    std::string greeting;
};

/**
 * 純函數：根據輸入資料回傳打招呼字串
 * 100% 確定性、無副作用
 */
inline Output solve(const Input& in) {
    return Output{"hello, " + in.name};
}

} // namespace solver
