# ZeroJudge Common I/O Library (`lib/`)

此共用庫專為 ZeroJudge 平台量身打造，旨在極致化輸入輸出速度，並統一處理 ZeroJudge 常見的三大測資情境：
1. **未指定測資筆數，讀取直到 EOF**（最常見，約 60% 題目）
2. **第一行給定測資總筆數 $T$，隨後有 $T$ 筆資料**
3. **以哨兵值結束（例如 `0` 或 `-1` 或 `0 0`）**

---

## 1. C++17 (`lib/cpp/zj_io.hpp`)

### 模式 A: 未指定測資筆數 (直到 EOF)
```cpp
#include "zj_io.hpp"

int main() {
    zj::FastIO io;
    int a, b;
    // read 會在遇到 EOF 時傳回 false
    while (io.read(a, b)) {
        int ans = a + b;
        io.println(ans);
    }
    return 0;
}
```

### 模式 B: 給定測資筆數 $T$
```cpp
#include "zj_io.hpp"

int main() {
    zj::FastIO io;
    int T;
    if (io.read(T)) {
        while (T--) {
            int n;
            io.read(n);
            // 解題核心邏輯
            io.println(n * 2);
        }
    }
    return 0;
}
```

### 模式 C: 哨兵值終止 (如 `n == 0`)
```cpp
#include "zj_io.hpp"

int main() {
    zj::FastIO io;
    int n;
    while (io.read(n) && n != 0) {
        io.println(n * n);
    }
    return 0;
}
```

---

## 2. C11 (`lib/c/zj_io.h`)

### 模式 A: 未指定測資筆數 (直到 EOF)
```c
#include "zj_io.h"

int main(void) {
    int a, b;
    while (zj_read_int(&a) && zj_read_int(&b)) {
        zj_write_int(a + b);
        zj_write_char('\n');
    }
    zj_flush();
    return 0;
}
```

---

## 3. Python 3.12 (`lib/python/zj_io.py`)

### 模式 A: Token 串流 (一次讀入切分，速度最快)
```python
from zj_io import TokenStream, fast_println

def main():
    stream = TokenStream()
    while stream.has_next():
        a = stream.next_int()
        b = stream.next_int()
        fast_println(a + b)

if __name__ == "__main__":
    main()
```

### 模式 B: 按行讀取 (字串含空白題型)
```python
from zj_io import LineStream, fast_println

def main():
    for line in LineStream.lines():
        fast_println(f"hello, {line}")

if __name__ == "__main__":
    main()
```

---

## 4. Java 11 (`lib/java/FastIO.java`)

### 模式 A: EOF 循環讀取
```java
public class Main {
    public static void main(String[] args) throws Exception {
        FastScanner scanner = new FastScanner();
        FastPrinter printer = new FastPrinter();

        while (scanner.hasNext()) {
            int a = scanner.nextInt();
            int b = scanner.nextInt();
            printer.println(a + b);
        }
        printer.flush();
    }
}
```
