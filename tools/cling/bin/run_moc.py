#!/usr/bin/env python3

from __future__ import annotations

import argparse
import pathlib
import shutil
import subprocess
import sys


HEADER_EXTENSIONS = {
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
}

SOURCE_EXTENSIONS = {
    ".cxx",
    ".cc",
    ".cpp",
    ".c++",
    ".cp",
}


def is_header_file(file_path: pathlib.Path) -> bool:
    return file_path.suffix.lower() in HEADER_EXTENSIONS


def is_source_file(file_path: pathlib.Path) -> bool:
    return file_path.suffix.lower() in SOURCE_EXTENSIONS


def is_supported_cpp_file(file_path: pathlib.Path) -> bool:
    return is_header_file(file_path) or is_source_file(file_path)


def contains_q_object(file_path: pathlib.Path) -> bool:
    try:
        file_text = file_path.read_text(encoding="utf-8")
        return "Q_OBJECT" in file_text
    except UnicodeDecodeError:
        file_text = file_path.read_text(encoding="utf-8", errors="ignore")
        return "Q_OBJECT" in file_text


def determine_output_file_path(input_file_path: pathlib.Path) -> pathlib.Path:
    base_name = input_file_path.stem

    if is_source_file(input_file_path):
        return input_file_path.with_name(f"{base_name}.moc")

    if is_header_file(input_file_path):
        return input_file_path.with_name(f"moc_{base_name}.cpp")

    raise ValueError(f"未対応のファイル種別です: {input_file_path}")


def find_moc_executable(specified_moc_path: str | None) -> str:
    if specified_moc_path:
        return specified_moc_path

    found_moc_path = shutil.which("moc")
    if found_moc_path:
        return found_moc_path

    raise FileNotFoundError(
        "moc が見つかりませんでした。"
        " PATH に追加するか、--moc で明示してください。"
    )


def run_moc(moc_executable_path: str, input_file_path: pathlib.Path, output_file_path: pathlib.Path) -> int:
    command_arguments = [
        moc_executable_path,
        str(input_file_path),
        "-o",
        str(output_file_path),
    ]

    completed_process = subprocess.run(command_arguments)
    return completed_process.returncode


def process_file(moc_executable_path: str, input_file_path: pathlib.Path) -> int:
    if not input_file_path.exists():
        print(f"ファイルが存在しません: {input_file_path}", file=sys.stderr)
        return 1

    if not input_file_path.is_file():
        print(f"通常のファイルではありません: {input_file_path}", file=sys.stderr)
        return 1

    if not is_supported_cpp_file(input_file_path):
        print(f"未対応の拡張子なのでスキップします: {input_file_path}")
        return 0

    if not contains_q_object(input_file_path):
        print(f"Q_OBJECT がないので何もしません: {input_file_path}")
        return 0

    output_file_path = determine_output_file_path(input_file_path)
    print(f"moc を実行します: {input_file_path} -> {output_file_path}")

    return_code = run_moc(moc_executable_path, input_file_path, output_file_path)

    if return_code == 0:
        print(f"生成しました: {output_file_path}")
    else:
        print(f"moc の実行に失敗しました: {input_file_path}", file=sys.stderr)

    return return_code


def collect_default_target_files() -> list[pathlib.Path]:
    current_directory = pathlib.Path.cwd()
    target_file_paths = []

    for file_path in sorted(current_directory.iterdir()):
        if file_path.is_file() and is_supported_cpp_file(file_path):
            target_file_paths.append(file_path)

    return target_file_paths


def main() -> int:
    argument_parser = argparse.ArgumentParser(
        description="Q_OBJECT を含む C++ ファイルに対してのみ moc を実行します。"
    )
    argument_parser.add_argument(
        "files",
        nargs="*",
        help="対象の C++ ヘッダーファイルまたは実装ファイル",
    )
    argument_parser.add_argument(
        "--moc",
        dest="moc_executable_path",
        default=None,
        help="moc 実行ファイルのパス",
    )

    parsed_arguments = argument_parser.parse_args()

    try:
        moc_executable_path = find_moc_executable(parsed_arguments.moc_executable_path)
    except FileNotFoundError as error:
        print(str(error), file=sys.stderr)
        return 1

    if parsed_arguments.files:
        input_file_paths = [pathlib.Path(file_name) for file_name in parsed_arguments.files]
    else:
        input_file_paths = collect_default_target_files()
        if not input_file_paths:
            print("カレントディレクトリーに対象の C++ ファイルがありません。")
            return 0

    overall_return_code = 0

    for input_file_path in input_file_paths:
        current_return_code = process_file(moc_executable_path, input_file_path)
        if current_return_code != 0:
            overall_return_code = current_return_code

    return overall_return_code


if __name__ == "__main__":
    raise SystemExit(main())
