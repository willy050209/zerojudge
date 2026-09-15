# ZeroJudge 極致效能優化與編譯環境規範

ZeroJudge 的評測環境對各題皆有精確的 **Time Limit (通常為 1.0s ~ 3.0s)** 與 **Memory Limit (通常為 64MB ~ 512MB)**。許多題目的測資量高達 $10^5 \sim 10^6$ 筆資料，一般常規寫法極易遭遇 **TLE (Time Limit Exceeded)**。

本手冊規範在四種支援語言下獲取最高執行效能的技巧。

---

## 1. 支援語言與官方環境

| 語言 | 編譯/執行指令 | 官方版本 | 推薦標籤 |
| :--- | :--- | :--- | :--- |
| **C** | `gcc -std=c11 -O2` | gcc 13.3.0 | 追求極致空間與最低常數時間 |
| **C++** | `g++ -std=c++17 -O2` | g++ 13.3.0 | 競技程式設計首選，STL 與抽象代價極低 |
| **Java** | OpenJDK 11 | 11.0.31 | 需嚴格防範 GC 停頓與 I/O 延遲 |
| **Python** | Python 3 | 3.12.3 | 需批次讀取 (TokenStream) 與避免 Python 內部迴圈開銷 |

### 1.1 本機開發環境實體路徑 (Local Environment)

| 工具 | 角色 | 本機實體路徑 |
| :--- | :--- | :--- |
| `g++` | C++17 編譯器 (MinGW 13.1.0) | `C:\Qt\Tools\mingw1310_64\bin\g++.exe` |
| `gcc` | C11 編譯器 (MinGW 13.1.0) | `C:\Qt\Tools\mingw1310_64\bin\gcc.exe` |
| `python` | Python 直譯器 (Python 3.13.1) | `C:\Python313\python.exe` |
| `javac` | Java 編譯器 (OpenJDK 25) | `C:\Program Files\Eclipse Adoptium\jdk-25.0.2.10-hotspot\bin\javac.exe` |
| `java` | Java 執行環境 (OpenJDK 25) | `C:\Program Files\Eclipse Adoptium\jdk-25.0.2.10-hotspot\bin\java.exe` |


---

## 2. C++17 極速優化原則 (g++ 13.3.0)

### 2.1 I/O 加速核心
- **基礎加速**：
  ```cpp
  std::cin.tie(nullptr);
  std::ios_base::sync_with_stdio(false);
  ```
- **禁止使用 `std::endl`**：`endl` 會強制清空輸出緩衝區 (flush)，比 `'\n'` 慢 10 倍以上。一律寫 `'\n'`。
- **大數據極速加速 ($N \ge 10^5$)**：
  使用 `lib/cpp/zj_io.hpp` 的 `zj::FastIO`，透過 64KB `fread` / `fwrite` 區塊讀取，速度比解綁後的 `cin/cout` 還快 2~4 倍。

### 2.2 記憶體與快取優化 (Cache Locality)
- **容器容量預分配**：使用 `std::vector` 時，若已知長度，務必呼叫 `vec.reserve(n)` 或直接指定大小 `vector<int> vec(n)`，避免反覆重新分配記憶體 (Reallocation)。
- **緊湊與扁平陣列 (Flat Arrays)**：
  優先使用一維陣列 `arr[i * cols + j]` 取代巢狀 `vector<vector<int>>`，能顯著提升 CPU L1/L2 快取命中率。
- **常數參考 (const reference)**：大型結構體或字串傳遞時，一律使用 `const T&`，禁止不必要的拷貝。

---

## 3. C11 極速優化原則 (gcc 13.3.0)

### 3.1 緩衝區 I/O
- 使用 `lib/c/zj_io.h` 中的 `zj_read_int()` 與 `zj_write_int()`。
- 相比 `scanf` / `printf`，自訂字元流解析省去了格式化字串剖析（Format string parsing）的額外開銷。

### 3.2 位元運算與緊湊結構
- 乘除 $2^k$ 優先使用 `<<` 與 `>>`。
- 取模 $2^k$ 優先使用 `& ((1 << k) - 1)`。
- 布林陣列使用 `uint64_t` 位元遮罩 (Bitset) 壓縮空間。

---

## 4. Python 3.12 極速優化原則 (Python 3.12.3)

Python 因直譯器與物件封裝開銷，常數時間最大，必須使用專門手法防止 TLE：

### 4.1 Token 串流讀取 (`sys.stdin.read().split()`)
- **致命瓶頸**：在迴圈中逐次呼叫 `input()` 或 `sys.stdin.readline()` 會反覆觸發 Python 呼叫鏈開銷。
- **極致解法**：使用 `lib/python/zj_io.py` 的 `TokenStream`，由底層 C 一次將整個標準輸入字串切片為 Token 陣列，隨後以索引游標讀取：
  ```python
  from zj_io import TokenStream
  stream = TokenStream()
  # 讀取全部資料
  while stream.has_next():
      val = stream.next_int()
  ```

### 4.2 遞迴深度解除
Python 預設遞迴上限僅 1000，樹狀 DP 或 DFS 題目必定拋出 `RecursionError`：
```python
import sys
sys.setrecursionlimit(1_000_000)
```

### 4.3 執行熱點 (Hotspot) 優化
- **區域變數快顯 (Local Variable Caching)**：Python 讀取區域變數 (`LOAD_FAST`) 比全域變數 (`LOAD_GLOBAL`) 快約 30%。在熱點迴圈前將模組函數綁定為區域別名：
  ```python
  append = my_list.append
  for x in data:
      append(x)
  ```
- **列表生成式 (List Comprehension)**：比顯式 `for` 迴圈快約 20~30%。

---

## 5. Java 11 極速優化原則 (OpenJDK 11.0.31)

### 5.1 嚴禁 `java.util.Scanner`
`Scanner` 內部大量使用正規表示式 (Regex) 解析，在 $10^5$ 筆整數輸入時耗時可達數秒，幾乎必得 TLE。
- 強制使用 `lib/java/FastIO.java` 中的 `FastScanner`。

### 5.2 輸出緩衝與 GC 減壓
- 輸出一律使用 `FastPrinter` (包裝 64KB `BufferedWriter`)，禁止在迴圈中頻繁呼叫 `System.out.println()`。
- **減少短暫物件建立**：在多筆測資迴圈中，避免每次 `new int[N]`，應建立固定上限陣列並重複利用（搭配顯式重設游標）。
