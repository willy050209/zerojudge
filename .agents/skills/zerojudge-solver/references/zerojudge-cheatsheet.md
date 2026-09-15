# ZeroJudge 評判系統特性與避坑指南

ZeroJudge 是一個以 Linux 為底層的線上評判系統 (Online Judge)，其評判機制與測資餵入方式有若干重要特性。

---

## 1. 評判結果代碼 (Verdicts) 與除錯對策

| 代碼 | 英文全名 | 中文意義 | 常見原因與除錯手段 |
| :--- | :--- | :--- | :--- |
| **AC** | Accepted | 完全正確 | 通過所有測試點。 |
| **WA** | Wrong Answer | 答案錯誤 | 1. 輸出格式錯誤（多餘空白或缺少換行）。<br/>2. 邊界條件未考慮（如 $N=0, 1$、負數、極值溢位）。<br/>3. 多筆測資變數未重置（**使用純函數即可徹底預防**）。 |
| **TLE** | Time Limit Exceeded | 超過時間限制 | 1. 未使用 Fast I/O（大量資料解析緩慢）。<br/>2. 演算法複雜度過高（例如 $O(N^2)$ 應用於 $N=10^5$）。<br/>3. EOF 條件未被正確捕捉，陷入無窮迴圈。 |
| **MLE** | Memory Limit Exceeded | 超過記憶體限制 | 1. 開了過大的靜態或全域陣列（超過限制如 64MB）。<br/>2. 遞迴過深造成 Call Stack 耗盡。<br/>3. 記憶體洩漏 (Memory Leak)。 |
| **OLE** | Output Limit Exceeded | 超過輸出限制 | 無窮迴圈印出字串，或 debug 輸出未移除。 |
| **RE** | Runtime Error | 執行時期錯誤 | 1. 陣列索引越界 (Index Out of Bounds)。<br/>2. 除以零 (Division by zero)。<br/>3. 遞迴爆棧 (Stack Overflow)。<br/>4. 指標存取非法記憶體 (Segmentation Fault)。 |
| **CE** | Compile Error | 編譯錯誤 | 使用了未支援的語法、缺少標頭檔，或依賴了本機特定環境。 |

---

## 2. ZeroJudge 的測資餵入與 EOF 慣例

ZeroJudge 使用標準管線 (`./solution < test.in > test.out`) 餵入測資。

### 關鍵陷阱：
1. **未給定測資筆數**：題目往往只說「有多筆輸入，每行包含兩個整數」，**絕不保證筆數**。
   - 正確處理：迴圈必須以讀取函數回傳的狀態或 EOF 為結束條件。
   - 錯誤示範：`for (int i = 0; i < 100; i++)` 或 `cin >> n; while (1)`。
2. **末尾空白與空行**：ZeroJudge 的測資末尾可能包含多個換行或空白字元。
   - 使用 `zj::FastIO` 或 `lib/python/zj_io.py` 的 TokenStream 會自動略過無效空白，安全可靠。

---

## 3. Java 送出規範

- ZeroJudge 在評測 Java 時，若類別為 `public class Main`，檔名通常儲存為 `Main.java`。
- 保守且最穩健的做法是：
  - 進入點類別若宣告為 `public class Main`，則檔名需為 `Main.java`。
  - 其餘輔助類別（如 `FastScanner`）**嚴格宣告為 package-private (不加 public)**，即可將所有類別無衝突地放置在同一個單一檔案中。

---

## 4. 單一檔案送出規範

ZeroJudge 線上提交框僅允許貼入**單一文字原始碼**。
- 本專案規範在 `src/` 中依據 SoC 原則模組化開發。
- 提交前**必須**執行 `python .agents/skills/zerojudge-solver/scripts/bundler.py <problem_id>`。
- 打包器會產出 `dist/<problem_id>_submission.<ext>`，此檔案包含所有相依項，可直接全選複製貼上 ZeroJudge。
