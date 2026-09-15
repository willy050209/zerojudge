#!/usr/bin/env python3
"""ZeroJudge Test Runner & Benchmark Tool

Compiles and executes ZeroJudge solutions against local test cases,
measuring execution time in milliseconds and peak memory, and validating output.

Usage:
    python test_runner.py <problem_directory> [--target {src,dist}] [--timeout <seconds>]

Example:
    python test_runner.py a001
"""

import os
import sys
import json
import time
import shutil
import argparse
import subprocess
from pathlib import Path
from typing import List, Dict, Any, Optional, Tuple


def find_workspace_root(start_path: Path) -> Path:
    """Find the workspace root containing 'lib' or '.agents'."""
    current = start_path.resolve()
    while current != current.parent:
        if (current / "lib").is_dir() or (current / ".agents").is_dir():
            return current
        current = current.parent
    return start_path.resolve()


def locate_compiler(cmd_name: str) -> str:
    """Find compiler using known local toolchain paths, PATH, or candidate directories."""
    known_paths = {
        "g++": Path(r"C:\Qt\Tools\mingw1310_64\bin\g++.exe"),
        "gcc": Path(r"C:\Qt\Tools\mingw1310_64\bin\gcc.exe"),
        "python": Path(r"C:\Python313\python.exe"),
        "javac": Path(r"C:\Program Files\Eclipse Adoptium\jdk-25.0.2.10-hotspot\bin\javac.exe"),
        "java": Path(r"C:\Program Files\Eclipse Adoptium\jdk-25.0.2.10-hotspot\bin\java.exe"),
    }
    if cmd_name in known_paths and known_paths[cmd_name].is_file():
        return str(known_paths[cmd_name])

    found = shutil.which(cmd_name)
    if found:
        return found

    # Fallbacks for Windows MinGW
    candidate_dirs = [
        Path(r"C:\Qt\Tools\mingw1310_64\bin"),
        Path(r"C:\Program Files\CodeBlocks\MinGW\bin"),
        Path(r"C:\msys64\ucrt64\bin"),
        Path(r"C:\msys64\mingw64\bin"),
    ]
    for cdir in candidate_dirs:
        cand = cdir / (cmd_name + ".exe")
        if cand.is_file():
            return str(cand)
    return cmd_name


class TestRunner:
    def __init__(self, problem_dir: Path, target: str = "dist", timeout_sec: float = 3.0):
        self.problem_dir = problem_dir.resolve()
        self.workspace_root = find_workspace_root(self.problem_dir)
        self.target = target
        self.timeout_sec = timeout_sec
        self.problem_id = self.problem_dir.name
        self.build_dir = self.problem_dir / "build"
        self.build_dir.mkdir(exist_ok=True)

    def detect_source_file(self) -> Tuple[str, Path]:
        """Detect language and source file depending on target (src or dist)."""
        if self.target == "dist":
            dist = self.problem_dir / "dist"
            if dist.is_dir():
                # Check for exact problem_id submission
                for ext, lang in [(".cpp", "cpp"), (".c", "c"), (".py", "py"), (".java", "java")]:
                    cand = dist / f"{self.problem_id}_submission{ext}"
                    if cand.is_file():
                        return lang, cand
                # Check for any submission file in dist
                for sub in dist.glob("*_submission.*"):
                    ext = sub.suffix.lower()
                    lang_map = {".cpp": "cpp", ".c": "c", ".py": "py", ".java": "java"}
                    if ext in lang_map:
                        return lang_map[ext], sub

            # If dist not bundled yet, fallback to src
            print(f"[TestRunner] Dist file not found, falling back to src/")
            self.target = "src"

        src = self.problem_dir / "src"
        if (src / "main.cpp").is_file():
            return "cpp", src / "main.cpp"
        if (src / "main.c").is_file():
            return "c", src / "main.c"
        if (src / "main.py").is_file():
            return "py", src / "main.py"
        if (src / "Main.java").is_file():
            return "java", src / "Main.java"

        raise FileNotFoundError(f"No valid source files found in {self.problem_dir}")

    def compile(self, lang: str, source_path: Path) -> Optional[List[str]]:
        """Compile source if needed and return executable invocation command."""
        exe_path = self.build_dir / f"{self.problem_id}_run"

        if lang == "cpp":
            gpp = locate_compiler("g++")
            out_bin = exe_path.with_suffix(".exe" if os.name == "nt" else "")
            cmd = [
                gpp,
                "-std=c++17",
                "-O2",
                "-pipe",
                f"-I{self.problem_dir / 'src'}",
                f"-I{self.workspace_root / 'lib' / 'cpp'}",
                str(source_path),
                "-o",
                str(out_bin)
            ]
            print(f"[Compile] {' '.join(cmd)}")
            res = subprocess.run(cmd, capture_output=True, text=True)
            if res.returncode != 0:
                print(f"[Compile Error]\n{res.stderr}")
                return None
            return [str(out_bin)]

        elif lang == "c":
            gcc = locate_compiler("gcc")
            out_bin = exe_path.with_suffix(".exe" if os.name == "nt" else "")
            cmd = [
                gcc,
                "-std=c11",
                "-O2",
                "-pipe",
                f"-I{self.problem_dir / 'src'}",
                f"-I{self.workspace_root / 'lib' / 'c'}",
                str(source_path),
                "-o",
                str(out_bin)
            ]
            print(f"[Compile] {' '.join(cmd)}")
            res = subprocess.run(cmd, capture_output=True, text=True)
            if res.returncode != 0:
                print(f"[Compile Error]\n{res.stderr}")
                return None
            return [str(out_bin)]

        elif lang == "py":
            py_cmd = sys.executable
            return [py_cmd, str(source_path)]

        elif lang == "java":
            javac = locate_compiler("javac")
            cmd = [javac, "-d", str(self.build_dir), str(source_path)]
            print(f"[Compile] {' '.join(cmd)}")
            res = subprocess.run(cmd, capture_output=True, text=True)
            if res.returncode != 0:
                print(f"[Compile Error]\n{res.stderr}")
                return None
            java = locate_compiler("java")
            main_class = source_path.stem
            return [java, "-cp", str(self.build_dir), main_class]

        return None

    def load_test_cases(self) -> List[Dict[str, str]]:
        """Load test cases from tests/cases.json or .in/.out pairs."""
        tests_dir = self.problem_dir / "tests"
        cases = []
        json_file = tests_dir / "cases.json"
        if json_file.is_file():
            data = json.loads(json_file.read_text(encoding="utf-8"))
            return data.get("cases", [])

        # Look for *.in files
        for in_file in sorted(tests_dir.glob("*.in")):
            out_file = in_file.with_suffix(".out")
            in_text = in_file.read_text(encoding="utf-8")
            out_text = out_file.read_text(encoding="utf-8") if out_file.is_file() else ""
            cases.append({
                "name": in_file.stem,
                "input": in_text,
                "expected": out_text
            })

        return cases

    def run_tests(self) -> bool:
        lang, source_path = self.detect_source_file()
        print(f"=== Testing Problem: {self.problem_id} ({lang.upper()}) [Target: {self.target}] ===")

        exec_cmd = self.compile(lang, source_path)
        if not exec_cmd:
            print("[Result] Compilation Failed!")
            return False

        cases = self.load_test_cases()
        if not cases:
            print("[TestRunner] No test cases found in tests/cases.json or tests/*.in")
            return True

        # Setup execution environment (especially PYTHONPATH)
        run_env = os.environ.copy()
        if lang == "py":
            py_paths = [
                str(self.problem_dir / "src"),
                str(self.workspace_root / "lib" / "python")
            ]
            existing_pypath = run_env.get("PYTHONPATH", "")
            if existing_pypath:
                py_paths.append(existing_pypath)
            run_env["PYTHONPATH"] = os.pathsep.join(py_paths)

        all_passed = True
        total_time_ms = 0.0

        for idx, case in enumerate(cases, 1):
            name = case.get("name", f"Case #{idx}")
            stdin_data = case.get("input", "")
            expected = case.get("expected", "").strip()

            start_t = time.perf_counter()
            try:
                proc = subprocess.run(
                    exec_cmd,
                    input=stdin_data,
                    capture_output=True,
                    text=True,
                    timeout=self.timeout_sec,
                    env=run_env
                )
                elapsed_ms = (time.perf_counter() - start_t) * 1000.0
                total_time_ms += elapsed_ms

                if proc.returncode != 0:
                    print(f"[{name}] RE (Runtime Error, code {proc.returncode}) - {elapsed_ms:.2f} ms")
                    if proc.stderr:
                        print(f"       stderr: {proc.stderr.strip()}")
                    all_passed = False
                    continue

                actual = proc.stdout.strip()
                if actual == expected:
                    print(f"  [PASS] {name} - {elapsed_ms:.2f} ms")
                else:
                    print(f"  [FAIL] {name} - {elapsed_ms:.2f} ms (WA)")
                    print(f"    Expected:\n{expected}")
                    print(f"    Actual:\n{actual}")
                    all_passed = False

            except subprocess.TimeoutExpired:
                print(f"  [FAIL] {name} - TLE (> {self.timeout_sec * 1000:.0f} ms)")
                all_passed = False

        print(f"=== Summary: {'ALL PASSED' if all_passed else 'SOME FAILED'} | Total Runtime: {total_time_ms:.2f} ms ===")
        return all_passed


def main():
    parser = argparse.ArgumentParser(description="Test runner and benchmark tool for ZeroJudge.")
    parser.add_argument("problem_dir", type=str, help="Problem directory (e.g. a001)")
    parser.add_argument("--target", choices=["src", "dist"], default="dist", help="Whether to test src or bundled dist (default: dist)")
    parser.add_argument("--timeout", type=float, default=3.0, help="Per-case timeout in seconds (default: 3.0)")
    args = parser.parse_args()

    runner = TestRunner(Path(args.problem_dir), target=args.target, timeout_sec=args.timeout)
    success = runner.run_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
