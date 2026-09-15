#pragma once
#include "zj_io.hpp"
#include "solver.hpp"

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
