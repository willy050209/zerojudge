#pragma once

namespace solver {

struct Input {
    int a;
    int b;
};

struct Output {
    int result;
};

/**
 * 核心純函數：
 * - 無 I/O 副作用
 * - 無外部或全域狀態依賴
 * - 保證確定性
 */
inline Output solve(const Input& in) {
    return Output{in.a + in.b};
}

} // namespace solver
