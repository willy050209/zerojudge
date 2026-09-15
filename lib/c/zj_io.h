#ifndef ZJ_IO_C_H
#define ZJ_IO_C_H

/**
 * ZeroJudge Common I/O Library for C11 (gcc -std=c11)
 * High-performance buffer I/O using standard fread/fwrite.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

#define ZJ_BUF_SIZE (1 << 16) // 64 KB

static char zj_in_buf[ZJ_BUF_SIZE];
static size_t zj_in_head = 0;
static size_t zj_in_tail = 0;
static bool zj_eof_reached = false;

static char zj_out_buf[ZJ_BUF_SIZE];
static size_t zj_out_tail = 0;

static inline char zj_next_char(void) {
    if (zj_in_head == zj_in_tail) {
        zj_in_head = 0;
        zj_in_tail = fread(zj_in_buf, 1, ZJ_BUF_SIZE, stdin);
        if (zj_in_head == zj_in_tail) {
            zj_eof_reached = true;
            return EOF;
        }
    }
    return zj_in_buf[zj_in_head++];
}

static inline bool zj_is_eof(void) {
    return zj_eof_reached;
}

static inline int zj_read_int(int *val) {
    char c = zj_next_char();
    if (c == EOF) return 0;
    while (c != EOF && c <= ' ') c = zj_next_char();
    if (c == EOF) return 0;

    bool neg = false;
    if (c == '-') {
        neg = true;
        c = zj_next_char();
    } else if (c == '+') {
        c = zj_next_char();
    }

    int res = 0;
    while (c >= '0' && c <= '9') {
        res = res * 10 + (c - '0');
        c = zj_next_char();
    }
    *val = neg ? -res : res;
    return 1;
}

static inline int zj_read_ll(long long *val) {
    char c = zj_next_char();
    if (c == EOF) return 0;
    while (c != EOF && c <= ' ') c = zj_next_char();
    if (c == EOF) return 0;

    bool neg = false;
    if (c == '-') {
        neg = true;
        c = zj_next_char();
    } else if (c == '+') {
        c = zj_next_char();
    }

    long long res = 0;
    while (c >= '0' && c <= '9') {
        res = res * 10 + (c - '0');
        c = zj_next_char();
    }
    *val = neg ? -res : res;
    return 1;
}

static inline int zj_read_char(char *val, bool skip_whitespace) {
    char c = zj_next_char();
    if (c == EOF) return 0;
    if (skip_whitespace) {
        while (c != EOF && isspace((unsigned char)c)) c = zj_next_char();
        if (c == EOF) return 0;
    }
    *val = c;
    return 1;
}

static inline int zj_read_str(char *buf) {
    char c = zj_next_char();
    if (c == EOF) return 0;
    while (c != EOF && isspace((unsigned char)c)) c = zj_next_char();
    if (c == EOF) return 0;

    int idx = 0;
    while (c != EOF && !isspace((unsigned char)c)) {
        buf[idx++] = c;
        c = zj_next_char();
    }
    buf[idx] = '\0';
    return 1;
}

static inline int zj_read_line(char *buf) {
    char c = zj_next_char();
    if (c == EOF) return 0;
    int idx = 0;
    while (c != EOF && c != '\n' && c != '\r') {
        buf[idx++] = c;
        c = zj_next_char();
    }
    buf[idx] = '\0';
    return 1;
}

static inline void zj_write_char(char c) {
    if (zj_out_tail == ZJ_BUF_SIZE) {
        fwrite(zj_out_buf, 1, ZJ_BUF_SIZE, stdout);
        zj_out_tail = 0;
    }
    zj_out_buf[zj_out_tail++] = c;
}

static inline void zj_write_int(int val) {
    if (val == 0) {
        zj_write_char('0');
        return;
    }
    if (val < 0) {
        zj_write_char('-');
        val = -val;
    }
    char temp[16];
    int len = 0;
    while (val > 0) {
        temp[len++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (int i = len - 1; i >= 0; --i) {
        zj_write_char(temp[i]);
    }
}

static inline void zj_write_ll(long long val) {
    if (val == 0) {
        zj_write_char('0');
        return;
    }
    if (val < 0) {
        zj_write_char('-');
        val = -val;
    }
    char temp[32];
    int len = 0;
    while (val > 0) {
        temp[len++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (int i = len - 1; i >= 0; --i) {
        zj_write_char(temp[i]);
    }
}

static inline void zj_write_str(const char *s) {
    while (*s) zj_write_char(*s++);
}

static inline void zj_flush(void) {
    if (zj_out_tail > 0) {
        fwrite(zj_out_buf, 1, zj_out_tail, stdout);
        zj_out_tail = 0;
    }
}

#endif // ZJ_IO_C_H
