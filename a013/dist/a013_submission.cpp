/* --- Bundled by ZeroJudge Solver Bundler --- */
#include <cstdio>
#include <cctype>
#include <string>
#include <iostream>
#include <utility>
#include <string_view>
#include <cstdlib>

/**
 * ZeroJudge Common I/O Library for C++17
 * Supports ultra-fast buffer I/O (fread/fwrite) and EOF detection loops.
 */


namespace zj {

/**
 * Standard I/O optimization: call once at the beginning of main()
 * if using standard cin/cout instead of buffer FastIO.
 */
inline void init_cin() {
    std::cin.tie(nullptr);
    std::ios_base::sync_with_stdio(false);
}

/**
 * High-performance buffer-based I/O reader and writer.
 * Suitable for large test cases (N >= 10^5).
 */
class FastIO {
private:
    static constexpr size_t BUF_SIZE = 1 << 16; // 64 KB
    char in_buf[BUF_SIZE];
    size_t in_head = 0, in_tail = 0;
    bool reached_eof = false;

    char out_buf[BUF_SIZE];
    size_t out_tail = 0;

    inline char next_char() {
        if (in_head == in_tail) {
            in_head = 0;
            in_tail = std::fread(in_buf, 1, BUF_SIZE, stdin);
            if (in_head == in_tail) {
                reached_eof = true;
                return EOF;
            }
        }
        return in_buf[in_head++];
    }

public:
    FastIO() = default;
    ~FastIO() { flush(); }

    // Check if EOF was encountered
    inline bool is_eof() const { return reached_eof; }

    // Read a single character (skipping whitespace by default)
    bool read_char(char& c, bool skip_whitespace = true) {
        char ch = next_char();
        if (ch == EOF) return false;
        if (skip_whitespace) {
            while (ch != EOF && std::isspace(static_cast<unsigned char>(ch))) {
                ch = next_char();
            }
            if (ch == EOF) return false;
        }
        c = ch;
        return true;
    }

    // Read signed/unsigned integer
    template <typename T>
    bool read_int(T& val) {
        char c = next_char();
        if (c == EOF) return false;
        while (c != EOF && c <= ' ') {
            c = next_char();
        }
        if (c == EOF) return false;

        bool neg = false;
        if constexpr (std::is_signed_v<T>) {
            if (c == '-') {
                neg = true;
                c = next_char();
            } else if (c == '+') {
                c = next_char();
            }
        }

        val = 0;
        while (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
            c = next_char();
        }

        if constexpr (std::is_signed_v<T>) {
            if (neg) val = -val;
        }
        return true;
    }

    // Read string (whitespace-delimited token)
    bool read_str(std::string& s) {
        s.clear();
        char c = next_char();
        if (c == EOF) return false;
        while (c != EOF && std::isspace(static_cast<unsigned char>(c))) {
            c = next_char();
        }
        if (c == EOF) return false;
        while (c != EOF && !std::isspace(static_cast<unsigned char>(c))) {
            s.push_back(c);
            c = next_char();
        }
        return true;
    }

    // Read line (stops at newline '\n' or EOF)
    bool read_line(std::string& s) {
        s.clear();
        char c = next_char();
        if (c == EOF) return false;
        while (c != EOF && c != '\n' && c != '\r') {
            s.push_back(c);
            c = next_char();
        }
        if (c == '\r') {
            char next = next_char();
            if (next != '\n' && next != EOF) {
                if (in_head > 0) in_head--; // push back 1 char if possible
            }
        }
        return true;
    }

    // Generic read overload
    template <typename T>
    bool read(T& val) {
        if constexpr (std::is_integral_v<T>) {
            return read_int(val);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return read_str(val);
        } else if constexpr (std::is_same_v<T, char>) {
            return read_char(val);
        } else {
            static_assert(!sizeof(T), "Unsupported type for FastIO::read");
        }
    }

    // Multi-argument read: returns true only if ALL arguments were read successfully
    template <typename T, typename... Args>
    bool read(T& first, Args&... rest) {
        if (!read(first)) return false;
        return read(rest...);
    }

    // Fast output writing
    inline void write_char(char c) {
        if (out_tail == BUF_SIZE) {
            std::fwrite(out_buf, 1, BUF_SIZE, stdout);
            out_tail = 0;
        }
        out_buf[out_tail++] = c;
    }

    template <typename T>
    void write_int(T val) {
        if (val == 0) {
            write_char('0');
            return;
        }
        if constexpr (std::is_signed_v<T>) {
            if (val < 0) {
                write_char('-');
                val = -val;
            }
        }
        char temp[32];
        int len = 0;
        while (val > 0) {
            temp[len++] = static_cast<char>('0' + (val % 10));
            val /= 10;
        }
        for (int i = len - 1; i >= 0; --i) {
            write_char(temp[i]);
        }
    }

    void write_str(const std::string& s) {
        for (char c : s) write_char(c);
    }

    void write_str(const char* s) {
        while (*s) write_char(*s++);
    }

    template <typename T>
    void print(const T& val) {
        if constexpr (std::is_integral_v<T>) {
            write_int(val);
        } else if constexpr (std::is_same_v<T, std::string>) {
            write_str(val);
        } else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
            write_str(val);
        } else if constexpr (std::is_same_v<T, char>) {
            write_char(val);
        }
    }

    template <typename T, typename... Args>
    void print(const T& first, const Args&... rest) {
        print(first);
        print(rest...);
    }

    template <typename... Args>
    void println(const Args&... args) {
        print(args...);
        write_char('\n');
    }

    void flush() {
        if (out_tail > 0) {
            std::fwrite(out_buf, 1, out_tail, stdout);
            out_tail = 0;
        }
    }
};

} // namespace zj

namespace solver {

struct Input {
    std::string s1;
    std::string s2;
};

struct Output {
    std::string diff_roman;
};

/**
 * 將單一羅馬數字符號轉為整數值
 */
inline constexpr int char_to_val(char c) {
    switch (c) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default:  return 0;
    }
}

/**
 * 純函數：將標準羅馬數字字串轉換為十進位整數
 */
inline int roman_to_int(std::string_view s) {
    int total = 0;
    const int n = static_cast<int>(s.size());
    for (int i = 0; i < n; ++i) {
        int cur = char_to_val(s[i]);
        if (i + 1 < n && cur < char_to_val(s[i + 1])) {
            total -= cur;
        } else {
            total += cur;
        }
    }
    return total;
}

/**
 * 純函數：將十進位整數轉換為標準羅馬數字字串 (0 輸出 "ZERO")
 */
inline std::string int_to_roman(int val) {
    if (val == 0) {
        return "ZERO";
    }

    static constexpr std::pair<int, const char*> ROMAN_TABLE[] = {
        {1000, "M"},
        {900,  "CM"},
        {500,  "D"},
        {400,  "CD"},
        {100,  "C"},
        {90,   "XC"},
        {50,   "L"},
        {40,   "XL"},
        {10,   "X"},
        {9,    "IX"},
        {5,    "V"},
        {4,    "IV"},
        {1,    "I"}
    };

    std::string res;
    res.reserve(16);
    for (const auto& [num, str] : ROMAN_TABLE) {
        while (val >= num) {
            res += str;
            val -= num;
        }
    }
    return res;
}

/**
 * 純函數：計算兩羅馬數字之差的絕對值，並轉為羅馬數字
 */
inline Output solve(const Input& in) {
    int val1 = roman_to_int(in.s1);
    int val2 = roman_to_int(in.s2);
    int diff = std::abs(val1 - val2);
    return Output{int_to_roman(diff)};
}

} // namespace solver

namespace io {

/**
 * 讀取下一筆測資。
 * 若遭遇 EOF 或終止符號 '#'，則回傳 false。
 */
inline bool read_input(zj::FastIO& io, solver::Input& in) {
    if (!io.read_str(in.s1)) {
        return false;
    }
    if (in.s1 == "#") {
        return false;
    }
    if (!io.read_str(in.s2)) {
        return false;
    }
    return true;
}

/**
 * 輸出結果並換行
 */
inline void write_output(zj::FastIO& io, const solver::Output& out) {
    io.println(out.diff_roman);
}

} // namespace io

int main() {
    zj::FastIO io;
    solver::Input in;

    // 多筆測資迴圈，直到 '#' 或 EOF 結束
    while (io::read_input(io, in)) {
        solver::Output out = solver::solve(in);
        io::write_output(io, out);
    }

    return 0;
}
