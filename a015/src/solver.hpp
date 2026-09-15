#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <immintrin.h>

namespace solver {

/**
 * 輸入資料結構：
 * rows: 矩陣列數 (M)
 * cols: 矩陣行數 (N)
 * data: 扁平化矩陣元素，大小為 rows * cols，以列優先 (Row-major) 存儲
 */
struct Input {
    int rows = 0;
    int cols = 0;
    std::vector<int> data;
};

/**
 * 輸出資料結構：
 * rows: 轉置後列數 (原 N)
 * cols: 轉置後行數 (原 M)
 * data: 扁平化轉置後矩陣元素，大小為 cols * rows
 */
struct Output {
    int rows = 0;
    int cols = 0;
    std::vector<int> data;
};

// 緩衝區最大維度：必須大於 100 且為 8 的整數倍（13 * 8 = 104）
constexpr int PADDED_DIM = 104;

/**
 * AVX2 8x8 32-bit 整數矩陣區塊轉置核心
 * 利用 256 位元向量暫存器進行暫存器層級置換
 */
__attribute__((target("avx2")))
inline void transpose8x8_avx2(const int* __restrict src, int src_stride,
                             int* __restrict dst, int dst_stride) {
    __m256i r0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 0 * src_stride));
    __m256i r1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 1 * src_stride));
    __m256i r2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 2 * src_stride));
    __m256i r3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 3 * src_stride));
    __m256i r4 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 4 * src_stride));
    __m256i r5 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 5 * src_stride));
    __m256i r6 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 6 * src_stride));
    __m256i r7 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + 7 * src_stride));

    // 階段 1：32 位元解交錯 (Unpack 32-bit)
    __m256i t0 = _mm256_unpacklo_epi32(r0, r1);
    __m256i t1 = _mm256_unpackhi_epi32(r0, r1);
    __m256i t2 = _mm256_unpacklo_epi32(r2, r3);
    __m256i t3 = _mm256_unpackhi_epi32(r2, r3);
    __m256i t4 = _mm256_unpacklo_epi32(r4, r5);
    __m256i t5 = _mm256_unpackhi_epi32(r4, r5);
    __m256i t6 = _mm256_unpacklo_epi32(r6, r7);
    __m256i t7 = _mm256_unpackhi_epi32(r6, r7);

    // 階段 2：64 位元解交錯 (Unpack 64-bit)
    __m256i u0 = _mm256_unpacklo_epi64(t0, t2);
    __m256i u1 = _mm256_unpackhi_epi64(t0, t2);
    __m256i u2 = _mm256_unpacklo_epi64(t1, t3);
    __m256i u3 = _mm256_unpackhi_epi64(t1, t3);
    __m256i u4 = _mm256_unpacklo_epi64(t4, t6);
    __m256i u5 = _mm256_unpackhi_epi64(t4, t6);
    __m256i u6 = _mm256_unpacklo_epi64(t5, t7);
    __m256i u7 = _mm256_unpackhi_epi64(t5, t7);

    // 階段 3：128 位元通道跨區排列 (Permute 128-bit Lanes)
    __m256i c0 = _mm256_permute2x128_si256(u0, u4, 0x20);
    __m256i c1 = _mm256_permute2x128_si256(u1, u5, 0x20);
    __m256i c2 = _mm256_permute2x128_si256(u2, u6, 0x20);
    __m256i c3 = _mm256_permute2x128_si256(u3, u7, 0x20);
    __m256i c4 = _mm256_permute2x128_si256(u0, u4, 0x31);
    __m256i c5 = _mm256_permute2x128_si256(u1, u5, 0x31);
    __m256i c6 = _mm256_permute2x128_si256(u2, u6, 0x31);
    __m256i c7 = _mm256_permute2x128_si256(u3, u7, 0x31);

    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 0 * dst_stride), c0);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 1 * dst_stride), c1);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 2 * dst_stride), c2);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 3 * dst_stride), c3);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 4 * dst_stride), c4);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 5 * dst_stride), c5);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 6 * dst_stride), c6);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + 7 * dst_stride), c7);
}

/**
 * SSE2 4x4 32-bit 整數矩陣區塊轉置降級核心
 * 所有 x86-64 處理器皆支援
 */
__attribute__((target("sse2")))
inline void transpose4x4_sse2(const int* __restrict src, int src_stride,
                             int* __restrict dst, int dst_stride) {
    __m128i r0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + 0 * src_stride));
    __m128i r1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + 1 * src_stride));
    __m128i r2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + 2 * src_stride));
    __m128i r3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + 3 * src_stride));

    __m128i t0 = _mm_unpacklo_epi32(r0, r1);
    __m128i t1 = _mm_unpackhi_epi32(r0, r1);
    __m128i t2 = _mm_unpacklo_epi32(r2, r3);
    __m128i t3 = _mm_unpackhi_epi32(r2, r3);

    __m128i c0 = _mm_unpacklo_epi64(t0, t2);
    __m128i c1 = _mm_unpackhi_epi64(t0, t2);
    __m128i c2 = _mm_unpacklo_epi64(t1, t3);
    __m128i c3 = _mm_unpackhi_epi64(t1, t3);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + 0 * dst_stride), c0);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + 1 * dst_stride), c1);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + 2 * dst_stride), c2);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + 3 * dst_stride), c3);
}

/**
 * AVX2 區塊轉置調度器
 */
__attribute__((target("avx2")))
inline void solve_with_avx2(const Input& in, Output& out) {
    alignas(32) int in_buf[PADDED_DIM * PADDED_DIM] = {0};
    alignas(32) int out_buf[PADDED_DIM * PADDED_DIM] = {0};

    // 填入資料並保持其餘邊界為 0
    for (int r = 0; r < in.rows; ++r) {
        std::memcpy(&in_buf[r * PADDED_DIM], &in.data[r * in.cols], sizeof(int) * in.cols);
    }

    // 每次以 8x8 區塊進行 SIMD 轉置
    for (int r = 0; r < in.rows; r += 8) {
        for (int c = 0; c < in.cols; c += 8) {
            transpose8x8_avx2(&in_buf[r * PADDED_DIM + c], PADDED_DIM,
                              &out_buf[c * PADDED_DIM + r], PADDED_DIM);
        }
    }

    // 複製有效範圍回傳
    out.rows = in.cols;
    out.cols = in.rows;
    out.data.resize(out.rows * out.cols);
    for (int r = 0; r < out.rows; ++r) {
        std::memcpy(&out.data[r * out.cols], &out_buf[r * PADDED_DIM], sizeof(int) * out.cols);
    }
}

/**
 * SSE2 區塊轉置調度器
 */
__attribute__((target("sse2")))
inline void solve_with_sse2(const Input& in, Output& out) {
    alignas(16) int in_buf[PADDED_DIM * PADDED_DIM] = {0};
    alignas(16) int out_buf[PADDED_DIM * PADDED_DIM] = {0};

    for (int r = 0; r < in.rows; ++r) {
        std::memcpy(&in_buf[r * PADDED_DIM], &in.data[r * in.cols], sizeof(int) * in.cols);
    }

    // 每次以 4x4 區塊進行 SIMD 轉置
    for (int r = 0; r < in.rows; r += 4) {
        for (int c = 0; c < in.cols; c += 4) {
            transpose4x4_sse2(&in_buf[r * PADDED_DIM + c], PADDED_DIM,
                              &out_buf[c * PADDED_DIM + r], PADDED_DIM);
        }
    }

    out.rows = in.cols;
    out.cols = in.rows;
    out.data.resize(out.rows * out.cols);
    for (int r = 0; r < out.rows; ++r) {
        std::memcpy(&out.data[r * out.cols], &out_buf[r * PADDED_DIM], sizeof(int) * out.cols);
    }
}

/**
 * 基礎純量 (Scalar) 矩陣轉置演算法（作為極端環境降級備份）
 */
inline Output solve_scalar(const Input& in) {
    Output out;
    out.rows = in.cols;
    out.cols = in.rows;
    out.data.resize(out.rows * out.cols);
    for (int r = 0; r < in.rows; ++r) {
        for (int c = 0; c < in.cols; ++c) {
            out.data[c * out.cols + r] = in.data[r * in.cols + c];
        }
    }
    return out;
}

/**
 * 核心轉置入口純函數 (SoC Core Function)
 * 根據硬體環境自動調用最佳化的 SIMD 實作
 */
inline Output solve(const Input& in) {
    if (in.rows == 0 || in.cols == 0) {
        return Output{0, 0, {}};
    }

    Output out;
    // 優先使用 AVX2 (若硬體支援)
    if (__builtin_cpu_supports("avx2")) {
        solve_with_avx2(in, out);
        return out;
    }
    // 次選 SSE2 (x86-64 標準支援)
    if (__builtin_cpu_supports("sse2")) {
        solve_with_sse2(in, out);
        return out;
    }
    // 基礎純函數
    return solve_scalar(in);
}

} // namespace solver
