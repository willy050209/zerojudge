#pragma once
#include "zj_io.hpp"
#include "solver.hpp"

namespace io {

/**
 * 讀取下一組矩陣測資。
 * 若遭遇 EOF 則回傳 false。
 */
inline bool read_input(zj::FastIO& io, solver::Input& in) {
    if (!io.read_int(in.rows) || !io.read_int(in.cols)) {
        return false;
    }
    int total = in.rows * in.cols;
    in.data.resize(total);
    for (int i = 0; i < total; ++i) {
        if (!io.read_int(in.data[i])) {
            return false;
        }
    }
    return true;
}

/**
 * 輸出翻轉後的矩陣。
 * 每列元素間以空格分隔，每列末端輸出換行。
 */
inline void write_output(zj::FastIO& io, const solver::Output& out) {
    for (int r = 0; r < out.rows; ++r) {
        for (int c = 0; c < out.cols; ++c) {
            io.write_int(out.data[r * out.cols + c]);
            if (c + 1 < out.cols) {
                io.write_char(' ');
            }
        }
        io.write_char('\n');
    }
}

} // namespace io
