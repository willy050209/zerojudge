#!/usr/bin/env python3
"""ZeroJudge Single-File Bundler

Combines modular source files (including local headers and lib/ dependencies)
into a single, self-contained submission file ready for ZeroJudge.

Usage:
    python bundler.py <problem_directory> [--lang {cpp,c,py,java}] [--output <output_path>]

Example:
    python bundler.py a001
"""

import os
import sys
import re
import argparse
from pathlib import Path
from typing import Set, List, Tuple


def find_workspace_root(start_path: Path) -> Path:
    """Find the workspace root containing 'lib' or '.agents'."""
    current = start_path.resolve()
    while current != current.parent:
        if (current / "lib").is_dir() or (current / ".agents").is_dir():
            return current
        current = current.parent
    return start_path.resolve()


class CppBundler:
    """Bundles C/C++ modular code into a single file by inlining local headers."""

    SYSTEM_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*<([^>]+)>\s*$')
    LOCAL_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*["\']([^"\']+)["\']\s*$')
    PRAGMA_ONCE_RE = re.compile(r'^\s*#\s*pragma\s+once\s*$')

    def __init__(self, workspace_root: Path, search_dirs: List[Path]):
        self.workspace_root = workspace_root
        self.search_dirs = search_dirs
        self.included_files: Set[Path] = set()
        self.system_includes: Set[str] = set()
        self.system_includes_order: List[str] = []

    def resolve_include(self, include_path_str: str, current_dir: Path) -> Path:
        """Locate an included file either relative to the current file or in search dirs."""
        cand = (current_dir / include_path_str).resolve()
        if cand.is_file():
            return cand

        for sdir in self.search_dirs:
            cand = (sdir / include_path_str).resolve()
            if cand.is_file():
                return cand

        # Check in workspace_root / lib / cpp
        cand = (self.workspace_root / "lib" / "cpp" / include_path_str).resolve()
        if cand.is_file():
            return cand

        cand = (self.workspace_root / "lib" / "c" / include_path_str).resolve()
        if cand.is_file():
            return cand

        raise FileNotFoundError(f"Cannot resolve local include: '{include_path_str}' from {current_dir}")

    def process_file(self, file_path: Path) -> List[str]:
        file_path = file_path.resolve()
        if file_path in self.included_files:
            return []
        self.included_files.add(file_path)

        lines = file_path.read_text(encoding="utf-8").splitlines()
        processed_body: List[str] = []

        for line in lines:
            if self.PRAGMA_ONCE_RE.match(line):
                continue

            sys_m = self.SYSTEM_INCLUDE_RE.match(line)
            if sys_m:
                hdr = sys_m.group(1)
                if hdr not in self.system_includes:
                    self.system_includes.add(hdr)
                    self.system_includes_order.append(hdr)
                continue

            loc_m = self.LOCAL_INCLUDE_RE.match(line)
            if loc_m:
                sub_file = self.resolve_include(loc_m.group(1), file_path.parent)
                sub_body = self.process_file(sub_file)
                processed_body.extend(sub_body)
                continue

            processed_body.append(line)

        return processed_body

    def bundle(self, entry_file: Path) -> str:
        body_lines = self.process_file(entry_file)
        result: List[str] = []
        result.append("/* --- Bundled by ZeroJudge Solver Bundler --- */")
        for hdr in self.system_includes_order:
            result.append(f"#include <{hdr}>")
        result.append("")
        result.extend(body_lines)
        return "\n".join(result) + "\n"


class PythonBundler:
    """Bundles modular Python solution and lib.python.zj_io into a single standalone script."""

    def __init__(self, workspace_root: Path):
        self.workspace_root = workspace_root

    def bundle(self, entry_file: Path, src_dir: Path) -> str:
        entry_text = entry_file.read_text(encoding="utf-8")
        bundled_parts: List[str] = []
        bundled_parts.append("#!/usr/bin/env python3")
        bundled_parts.append("# --- Bundled by ZeroJudge Solver Bundler ---")

        # Check if zj_io is imported
        needs_zj_io = ("zj_io" in entry_text) or any("zj_io" in f.read_text(encoding="utf-8") for f in src_dir.glob("*.py"))

        if needs_zj_io:
            zj_io_path = self.workspace_root / "lib" / "python" / "zj_io.py"
            if zj_io_path.is_file():
                zj_io_code = zj_io_path.read_text(encoding="utf-8")
                # Remove module docstring
                zj_io_clean = re.sub(r'^"""[\s\S]*?"""\n?', '', zj_io_code)
                bundled_parts.append("\n# --- BEGIN: Embedded zj_io.py ---")
                bundled_parts.append(zj_io_clean)
                bundled_parts.append("# --- END: Embedded zj_io.py ---\n")

        # Inline other modules in src_dir (except entry_file itself)
        for py_file in sorted(src_dir.glob("*.py")):
            if py_file.resolve() == entry_file.resolve():
                continue
            content = py_file.read_text(encoding="utf-8")
            # Strip imports of zj_io or internal modules
            content = re.sub(r'^\s*(from\s+(\.?\w+|\w+\.\w+)\s+)?import\s+zj_io.*$', '', content, flags=re.MULTILINE)
            content = re.sub(r'^\s*from\s+\.\s*import.*$', '', content, flags=re.MULTILINE)
            bundled_parts.append(f"\n# --- BEGIN: {py_file.name} ---")
            bundled_parts.append(content)
            bundled_parts.append(f"# --- END: {py_file.name} ---\n")

        # Clean entry_file imports
        clean_entry = re.sub(r'^\s*(from\s+(\.?\w+|\w+\.\w+)\s+)?import\s+zj_io.*$', '', entry_text, flags=re.MULTILINE)
        clean_entry = re.sub(r'^\s*from\s+\.\s*import.*$', '', clean_entry, flags=re.MULTILINE)
        bundled_parts.append("\n# --- Main Solution ---")
        bundled_parts.append(clean_entry)

        return "\n".join(bundled_parts) + "\n"


class JavaBundler:
    """Bundles Java modular code into a single file with one public/solution class."""

    def __init__(self, workspace_root: Path):
        self.workspace_root = workspace_root

    def bundle(self, entry_file: Path, src_dir: Path) -> str:
        entry_text = entry_file.read_text(encoding="utf-8")
        bundled_parts: List[str] = []
        bundled_parts.append("// --- Bundled by ZeroJudge Solver Bundler ---")

        # Collect imports
        import_lines: Set[str] = set()
        import_order: List[str] = []

        def extract_imports(text: str) -> str:
            clean_lines = []
            for line in text.splitlines():
                if line.strip().startswith("package "):
                    continue
                if line.strip().startswith("import "):
                    imp = line.strip()
                    if imp not in import_lines and "lib.java" not in imp:
                        import_lines.add(imp)
                        import_order.append(imp)
                    continue
                clean_lines.append(line)
            return "\n".join(clean_lines)

        # Include FastIO if referenced
        fastio_path = self.workspace_root / "lib" / "java" / "FastIO.java"
        fastio_code = ""
        if fastio_path.is_file():
            fastio_clean = extract_imports(fastio_path.read_text(encoding="utf-8"))
            fastio_code = fastio_clean

        clean_entry = extract_imports(entry_text)

        # Build output
        for imp in import_order:
            bundled_parts.append(imp)
        bundled_parts.append("")

        if fastio_code:
            bundled_parts.append("// --- Embedded FastIO ---")
            bundled_parts.append(fastio_code)
            bundled_parts.append("")

        bundled_parts.append("// --- Solution ---")
        bundled_parts.append(clean_entry)

        return "\n".join(bundled_parts) + "\n"


def bundle_project(problem_dir: Path, lang: str = None, output_file: Path = None) -> Path:
    problem_dir = problem_dir.resolve()
    if not problem_dir.is_dir():
        raise NotADirectoryError(f"Directory not found: {problem_dir}")

    workspace_root = find_workspace_root(problem_dir)
    src_dir = problem_dir / "src"
    dist_dir = problem_dir / "dist"
    dist_dir.mkdir(parents=True, exist_ok=True)
    problem_id = problem_dir.name

    # Detect language and entry file
    if not lang:
        if (src_dir / "main.cpp").is_file():
            lang = "cpp"
        elif (src_dir / "main.c").is_file():
            lang = "c"
        elif (src_dir / "main.py").is_file():
            lang = "py"
        elif (src_dir / "Main.java").is_file():
            lang = "java"
        else:
            raise FileNotFoundError(f"Could not automatically detect entry file in {src_dir}")

    search_dirs = [src_dir, workspace_root / "lib" / "cpp", workspace_root / "lib" / "c"]

    if lang == "cpp":
        entry = src_dir / "main.cpp"
        out_path = output_file or (dist_dir / f"{problem_id}_submission.cpp")
        bundler = CppBundler(workspace_root, search_dirs)
        content = bundler.bundle(entry)
    elif lang == "c":
        entry = src_dir / "main.c"
        out_path = output_file or (dist_dir / f"{problem_id}_submission.c")
        bundler = CppBundler(workspace_root, search_dirs)
        content = bundler.bundle(entry)
    elif lang == "py":
        entry = src_dir / "main.py"
        out_path = output_file or (dist_dir / f"{problem_id}_submission.py")
        bundler = PythonBundler(workspace_root)
        content = bundler.bundle(entry, src_dir)
    elif lang == "java":
        entry = src_dir / "Main.java"
        out_path = output_file or (dist_dir / f"{problem_id}_submission.java")
        bundler = JavaBundler(workspace_root)
        content = bundler.bundle(entry, src_dir)
    else:
        raise ValueError(f"Unsupported language: {lang}")

    out_path.write_text(content, encoding="utf-8")
    print(f"[Bundler] Successfully bundled {lang.upper()} project into: {out_path}")
    return out_path


def main():
    parser = argparse.ArgumentParser(description="Bundle modular ZeroJudge solution into a single file.")
    parser.add_argument("problem_dir", type=str, help="Problem project directory (e.g. a001)")
    parser.add_argument("--lang", choices=["cpp", "c", "py", "java"], help="Target language (default: auto-detect)")
    parser.add_argument("--output", type=str, help="Custom output file path")
    args = parser.parse_args()

    out = bundle_project(Path(args.problem_dir), args.lang, Path(args.output) if args.output else None)
    print(f"Output size: {out.stat().st_size} bytes")


if __name__ == "__main__":
    main()
