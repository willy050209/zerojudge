"""ZeroJudge Common I/O Library for Python 3.12

High-performance token and line streams, EOF handling, and fast stdout printing.
"""

import sys
from typing import Iterator, List, Optional, Tuple, Any


def set_fast_recursion(limit: int = 1_000_000) -> None:
    """Increase recursion depth limit for deep DFS/tree traversals."""
    sys.setrecursionlimit(limit)


class TokenStream:
    """Fast token reader utilizing sys.stdin.read().split().

    This is the fastest approach in Python for competitive programming when
    all input fits in memory.
    """

    def __init__(self, content: Optional[str] = None):
        if content is not None:
            self._tokens = content.split()
        else:
            raw = sys.stdin.read()
            self._tokens = raw.split() if raw else []
        self._idx = 0
        self._total = len(self._tokens)

    def has_next(self) -> bool:
        return self._idx < self._total

    def next_str(self) -> str:
        if self._idx >= self._total:
            raise EOFError("No more tokens in input stream.")
        val = self._tokens[self._idx]
        self._idx += 1
        return val

    def next_int(self) -> int:
        return int(self.next_str())

    def next_float(self) -> float:
        return float(self.next_str())

    def next_ints(self, count: int) -> List[int]:
        res = [int(x) for x in self._tokens[self._idx : self._idx + count]]
        self._idx += count
        return res

    def remaining_tokens(self) -> List[str]:
        res = self._tokens[self._idx :]
        self._idx = self._total
        return res


class LineStream:
    """Fast line-by-line reader for problems where whitespace within lines is significant."""

    @staticmethod
    def lines() -> Iterator[str]:
        for line in sys.stdin:
            yield line.rstrip("\r\n")

    @staticmethod
    def read_line() -> Optional[str]:
        line = sys.stdin.readline()
        if not line:
            return None
        return line.rstrip("\r\n")


def fast_print(*args: Any, sep: str = " ", end: str = "\n") -> None:
    """High performance stdout write wrapper."""
    sys.stdout.write(sep.join(map(str, args)) + end)


def fast_println(*args: Any) -> None:
    """Fast stdout write with space separation and newline."""
    fast_print(*args, sep=" ", end="\n")
