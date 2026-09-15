#pragma once
#include "zj_io.hpp"
#include "solver.hpp"

namespace io {

/**
 * 負責從標準輸入讀取並解析為純資料結構
 * 回傳 false 代表遇到 EOF
 */
inline bool read_input(zj::FastIO& io, solver::Input& in) {
    return io.read(in.a, in.b);
}

/**
 * 負責將純資料結構輸出至標準輸出
 */
inline void write_output(zj::FastIO& io, const solver::Output& out) {
    io.println(out.result);
}

} // namespace io
