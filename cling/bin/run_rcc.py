#!/usr/bin/env python3

from __future__ import annotations

import argparse
import pathlib
import shutil
import subprocess
import sys


TARGET_EXTENSION = ".qrc"


def find_rcc_executable(specified_rcc_path: str | None) -> str:
    if specified_rcc_path:
        return specified_rcc_path

    found_rcc_path = shutil.which("rcc")
    if found_rcc_path:
        return found_rcc_path

    raise FileNotFoundError(
        "rcc が見つかりませんでした。"
        " PATH に追加するか、--rcc で明示してください。"
    )


def is_qrc_file(file_path: pathlib.Path) -> bool:
    return file_path.suffix.lower() == TARGET_EXTENSION


def determine_output_file_path(input_file_path: pathlib.Path) -> pathlib.Path:
    base_name = input_file_path.stem
    return input_file_path.with_name(f"qrc_{base_name}.cpp")


def run_rcc(
    rcc_executable_path: str,
    input_file_path: pathlib.Path,
    output_file_path: pathlib.Path,
) -> int:
    command_arguments = [
        rcc_executable_path,
        str(input_file_path),
        "-o",
        str(output_file_path),
    ]

    completed_process = subprocess.run(command_arguments)
    return completed_process.returncode


def process_file(rcc_executable_path: str, input_file_path: pathlib.Path) -> int:
    if not input_file_path.exists():
        print(f"ファイルが存在しません: {input_file_path}", file=sys.stderr)
        return 1

    if not input_file_path.is_file():
        print(f"通常のファイルではありません: {input_file_path}", file=sys.stderr)
        return 1

    if not is_qrc_file(input_file_path):
        print(f".qrc ではないので何もしません: {input_file_path}")
        return 0

    output_file_path = determine_output_file_path(input_file_path)
    print(f"rcc を実行します: {input_file_path} -> {output_file_path}")

    return_code = run_rcc(rcc_executable_path, input_file_path, output_file_path)

    if return_code == 0:
        print(f"生成しました: {output_file_path}")
    else:
        print(f"rcc の実行に失敗しました: {input_file_path}", file=sys.stderr)

    return return_code


def collect_default_target_files() -> list[pathlib.Path]:
    current_directory = pathlib.Path.cwd()
    target_file_paths = []

    for file_path in sorted(current_directory.iterdir()):
        if file_path.is_file() and is_qrc_file(file_path):
            target_file_paths.append(file_path)

    return target_file_paths


def main() -> int:
    argument_parser = argparse.ArgumentParser(
        description="指定した .qrc ファイルに対して rcc を実行し、qrc_<basename>.cpp を生成します。"
    )
    argument_parser.add_argument(
        "files",
        nargs="*",
        help="対象ファイル",
    )
    argument_parser.add_argument(
        "--rcc",
        dest="rcc_executable_path",
        default=None,
        help="rcc 実行ファイルのパス",
    )

    parsed_arguments = argument_parser.parse_args()

    try:
        rcc_executable_path = find_rcc_executable(parsed_arguments.rcc_executable_path)
    except FileNotFoundError as error:
        print(str(error), file=sys.stderr)
        return 1

    if parsed_arguments.files:
        input_file_paths = [pathlib.Path(file_name) for file_name in parsed_arguments.files]
    else:
        input_file_paths = collect_default_target_files()
        if not input_file_paths:
            print("カレントディレクトリーに .qrc ファイルがありません。")
            return 0

    overall_return_code = 0

    for input_file_path in input_file_paths:
        current_return_code = process_file(rcc_executable_path, input_file_path)
        if current_return_code != 0:
            overall_return_code = current_return_code

    return overall_return_code


if __name__ == "__main__":
    raise SystemExit(main())
