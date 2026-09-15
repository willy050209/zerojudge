from typing import NamedTuple

class InputData(NamedTuple):
    a: int
    b: int

class OutputData(NamedTuple):
    result: int

def solve(data: InputData) -> OutputData:
    """純函數：無任何 I/O 或外部狀態依賴"""
    return OutputData(result=data.a + data.b)
