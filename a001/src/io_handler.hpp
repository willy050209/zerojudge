#pragma once
#include "zj_io.hpp"
#include "solver.hpp"

namespace io {

/**
 * 讀取下一筆測資，若到達 EOF 則傳回 false
 */
inline bool read_input(zj::FastIO& io, solver::Input& in) {
    return io.read_str(in.name);
}

/**
 * 格式化輸出
 */
inline void write_output(zj::FastIO& io, const solver::Output& out) {
    io.println(out.greeting);
}

} // namespace io
