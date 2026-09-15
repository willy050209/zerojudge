from zj_io import TokenStream, fast_println
from solver import InputData, solve

def main():
    stream = TokenStream()
    while stream.has_next():
        a = stream.next_int()
        b = stream.next_int()
        in_data = InputData(a=a, b=b)
        out_data = solve(in_data)
        fast_println(out_data.result)

if __name__ == "__main__":
    main()
