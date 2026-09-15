# SoC (關注點分離) 與純函數設計模式在 ZeroJudge 的實踐

在競技程式競賽 (Competitive Programming) 與線上評判系統 (Online Judge) 中，傳統寫法往往將 I/O、演算法邏輯與狀態變數全部擠在一個全域範圍或 `main()` 函式內。這種做法存在嚴重缺陷：
1. **多筆測資 (Multiple Test Cases) 污染**：前一筆測資的全域陣列或計數器未清空，導致本地範例通過，但提交 ZeroJudge 卻出現 WA (Wrong Answer)。
2. **無法針對核心邏輯做單元測試**：若要測試演算法邊界，必須模擬標準輸入輸出 (stdin/stdout)。
3. **除錯與推導困難**：I/O 解析邏輯與複雜的動態規劃或圖論演算法混雜。

本專案全面推行 **SoC 原則 (Separation of Concerns)** 與 **純函數優先 (Pure Functions First)**。

---

## 1. 架構三層分工

| 層級 | 名稱 | 職責 | 特性 |
| :--- | :--- | :--- | :--- |
| 1 | **I/O 處理層 (Input Layer)** | 負責從標準輸入讀取資料、處理 EOF、格式拆解，呼叫 `lib/` 極速 I/O。 | 有副作用（讀取 stdin），但不包含任何題目演算法。輸出結構化資料物件。 |
| 2 | **核心解題層 (Core Solver)** | 100% 純函數 (Pure Function)。輸入純資料結構，計算並返回解答純資料結構。 | **無副作用、無全域狀態、確定性**。完全獨立於 I/O，可直接做邊界單元測試。 |
| 3 | **格式輸出層 (Output Layer)** | 接收純函數返回的資料結構，格式化後透過 `lib/` 快速輸出至標準輸出。 | 有副作用（寫入 stdout），但不改變解答資料。 |

---

## 2. 各語言實踐範例

### 2.1 C++17 實踐模式

#### `src/solver.hpp` (純函數核心)
```cpp
#pragma once
#include <string>

namespace solver {

struct Input {
    std::string name;
};

struct Output {
    std::string greeting;
};

// 純函數：輸入確定的 Input，回傳確定的 Output，無任何 I/O 或全域變數
inline Output solve(const Input& in) {
    return Output{"hello, " + in.name};
}

} // namespace solver
```

#### `src/io_handler.hpp` (I/O 解析與轉換)
```cpp
#pragma once
#include "zj_io.hpp"
#include "solver.hpp"

namespace io {

inline bool read_next(zj::FastIO& io, solver::Input& in) {
    return io.read(in.name);
}

inline void write_output(zj::FastIO& io, const solver::Output& out) {
    io.println(out.greeting);
}

} // namespace io
```

#### `src/main.cpp` (進入點組裝)
```cpp
#include "zj_io.hpp"
#include "solver.hpp"
#include "io_handler.hpp"

int main() {
    zj::FastIO io;
    solver::Input in;
    while (io::read_next(io, in)) {
        solver::Output out = solver::solve(in);
        io::write_output(io, out);
    }
    return 0;
}
```

---

### 2.2 Python 3.12 實踐模式

#### `src/solver.py` (純函數)
```python
from typing import NamedTuple

class InputData(NamedTuple):
    name: str

class OutputData(NamedTuple):
    greeting: str

def solve(data: InputData) -> OutputData:
    """純函數：無副作用，給定相同輸入必得相同輸出"""
    return OutputData(greeting=f"hello, {data.name}")
```

#### `src/main.py` (組裝進入點)
```python
from zj_io import LineStream, fast_println
from solver import InputData, solve

def main():
    for line in LineStream.lines():
        in_data = InputData(name=line)
        out_data = solve(in_data)
        fast_println(out_data.greeting)

if __name__ == "__main__":
    main()
```

---

## 3. 純函數對多筆測資與極限測資的巨大優勢

1. **零污染保證**：
   在純函數架構中，所有運算狀態（如 DP 陣列、狀態標記）都在函數內部重新建立，或作為參數傳遞，徹底杜絕 ZeroJudge 上因多筆測資共用未初始化全域陣列所引發的 WA。
2. **極限測資自動化生成 (Stress Testing)**：
   代理 AI 可以輕易撰寫隨機測資腳本，直接以 `solve(generate_random_case())` 批次呼叫，比對暴力法 (Brute-Force) 與最佳化演算法的結果。
