#pragma once
/**
 * ZeroJudge Common I/O Library for C++17
 * Supports ultra-fast buffer I/O (fread/fwrite) and EOF detection loops.
 */

#include <cstdio>
#include <cctype>
#include <string>
#include <iostream>
#include <utility>

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
