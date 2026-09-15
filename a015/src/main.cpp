#include "zj_io.hpp"
#include "solver.hpp"
#include "io_handler.hpp"

int main() {
    zj::FastIO io;
    solver::Input in;

    // 多筆測資連續處理，直到 EOF 結束
    while (io::read_input(io, in)) {
        solver::Output out = solver::solve(in);
        io::write_output(io, out);
    }

    return 0;
}
