# a015: 矩陣的翻轉

## 1. 題目理解與規格分析
- **題目連結**：ZeroJudge [a015: 矩陣的翻轉](https://zerojudge.tw/ShowProblem?problemid=a015)
- **題目描述**：
  給定一個 $M \times N$ 的矩陣 $A$（$M$ 為列數，$N$ 為行數），求其轉置矩陣 $A^T$（將原本的行與列對調，維度變為 $N \times M$）。
- **輸入規格**：
  每個輸入檔包含多組測試資料，持續讀取直到檔案結尾（EOF）。
  每組測試資料第一行為兩個正整數 $M$（列數，row $< 100$）與 $N$（行數，column $< 100$）。
  接著有 $M$ 行，每行包含 $N$ 個整數，代表矩陣元素。
- **輸出規格**：
  對每組輸入，輸出翻轉後的 $N \times M$ 矩陣，共 $N$ 行，每行 $M$ 個以空白分隔的整數。
- **限制條件**：
  - $M < 100, N < 100$
  - 時間限制：1.0 秒
  - 記憶體限制：64 MB
  - **使用者明確指定：請使用 SIMD 指令集進行運算加速**

---

## 2. 演算法設計與 SIMD 向量化推導

### 2.1 轉置問題本質與記憶體存取瓶頸
在標準二維陣列的純純純迴圈轉置中：
$$B[j][i] = A[i][j]$$
- 讀取 $A[i][j]$ 為行連續（Row-major cache friendly），但寫入 $B[j][i]$ 會造成大跨距（Stride）記憶體寫入，引發 Cache Miss 與寫入緩衝區堵塞。
- 現代 CPU 具備 SIMD（Single Instruction, Multiple Data）指令集，可一次載入多個連續元素並利用向量暫存器內的置換（Shuffle/Unpack/Permute）指令在暫存器層級完成轉置，再連續寫出至記憶體。

### 2.2 AVX2 8x8 區塊轉置演算法 (AVX2 256-bit Vectorization)
針對 32 位元整數（`int32_t`），AVX2 向量暫存器（256-bit）可容納 8 個整數。
將整個矩陣劃分為 $8 \times 8$ 的小區塊（Tile/Block），對每個 $8 \times 8$ 區塊透過三階段的暫存器置換完成轉置：

1. **第 1 階段：32 位元解交錯 (Unpack 32-bit)**
   利用 `_mm256_unpacklo_epi32` 與 `_mm256_unpackhi_epi32`，分別對相鄰兩列 $(r_0, r_1), (r_2, r_3), (r_4, r_5), (r_6, r_7)$ 進行 32-bit 交錯組合。
2. **第 2 階段：64 位元解交錯 (Unpack 64-bit)**
   利用 `_mm256_unpacklo_epi64` 與 `_mm256_unpackhi_epi64`，將第 1 階段的中間結果以 64-bit 為單位進行二次交錯。
3. **第 3 階段：128 位元通道跨區排列 (Permute 128-bit Lanes)**
   AVX2 大多數指令限制在 128-bit Lane 內部操作，最後使用 `_mm256_permute2x128_si256` 跨通道重組低半部與高半部，分別組合成第 0 到 7 行的最終轉置向量，直接寫回記憶體。

### 2.3 任意尺寸矩陣邊界處理 (Padding / Tiling)
由於 $M, N < 100$，最大不超過 100：
- 我們預留 $104 \times 104$（104 為 8 的倍數，$13 \times 8 = 104$）的對齊靜態緩衝區。
- 將原始 $M \times N$ 矩陣複製至 $104 \times 104$ 緩衝區（邊界未滿 8 之填補區域為 0）。
- 每次步進 8 進行 $8 \times 8$ 向量轉置，確保每一條 SIMD Load/Store 均為合法記憶體且符合 32-byte 邊界對齊。
- 轉置完成後，依據 $N \times M$ 的維度讀取有效區間輸出，邊界邏輯單純且完全無條件分支分歧（Branchless）。

### 2.4 多層次降級與 CPU 相容性 (Dynamic CPU Dispatch)
- 優先檢測 `__builtin_cpu_supports("avx2")`：
  - 若支援，使用 AVX2 8x8 核心；
  - 若不支援，降級至 SSE2 4x4 核心（x86-64 架構保證 100% 支援）；
  - 亦提供 Scalar 純純迴圈作為參考基準。

### 2.5 關注點分離 (SoC) 與純函數架構設計
- **核心層 (`solver.hpp`)**：
  - 定義 `solver::Input`（含 `rows`, `cols`, `std::vector<int> data`）與 `solver::Output`。
  - `solver::solve` 為 100% 純函數，內部計算無副作用、無全域殘留。
  - 封裝 `transpose8x8_avx2`、`transpose4x4_sse2` 與分塊調度。
- **I/O 處理層 (`io_handler.hpp`)**：
  - 使用 `zj::FastIO` 進行緩衝讀寫（`fread`/`fwrite`），高效處理 EOF 迴圈及大批測資。
  - 不包含任何矩陣轉置演算法細節。
- **進入點 (`main.cpp`)**：
  - 負責串接 I/O 與解題器，清晰精煉。

---

## 3. 複雜度分析
- **時間複雜度**：
  - 矩陣維度最大 $100 \times 100 = 10^4$ 個元素。
  - 區塊數量最多 $\lceil 100 / 8 \rceil \times \lceil 100 / 8 \rceil = 13 \times 13 = 169$ 個。
  - 每個 $8 \times 8$ 區塊僅需約 24 個 SIMD 指令（8 次 Load、16 次 Unpack/Permute、8 次 Store）。
  - 單筆測資 SIMD 運算耗時不到 1 微秒（$\approx 0.001$ ms），整體時間複雜度為嚴格的 $O(M \times N)$，且常數極小（SIMD 加速比純純純搬移提升數倍）。
- **空間複雜度**：
  - 暫存緩衝區 $104 \times 104 \times 4 \text{ Bytes} \approx 43.2 \text{ KB}$，完全置於 L1 Data Cache 內，空間複雜度 $O(M \times N)$。

---

## 4. 邊界測試案例
1. **範例案例**：$2 \times 3$ 矩陣，轉置後為 $3 \times 2$。
2. **單一元素**：$1 \times 1$ 極小矩陣。
3. **單行向量**：$1 \times 5$ 矩陣轉為 $5 \times 1$。
4. **單列向量**：$4 \times 1$ 矩陣轉為 $1 \times 4$。
5. **8的整數倍**：$8 \times 8$ 矩陣，完整匹配 SIMD 區塊。
6. **非8的整數倍**：$9 \times 9$ 矩陣，考驗跨邊界 Padding 處理。
7. **極大邊界**：接近上限的 $99 \times 99$ 矩陣，包含負數與零。
8. **多測資連續串流**：連續多筆測資 EOF 正常中止。
