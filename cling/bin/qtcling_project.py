#!/usr/bin/env python3

from __future__ import annotations

import argparse
import pathlib
import re
import sys
from dataclasses import dataclass, field

try:
    import tomllib
except ModuleNotFoundError:  # pragma: no cover
    tomllib = None


DEFAULT_WIDGET_MODULES = ["Core", "Gui", "Widgets"]
DEFAULT_CORE_MODULES = ["Core"]

MODULE_NAME_MAP = {
    "core": "Core",
    "gui": "Gui",
    "widgets": "Widgets",
    "network": "Network",
    "printsupport": "PrintSupport",
    "concurrent": "Concurrent",
    "qml": "Qml",
    "quick": "Quick",
    "quickwidgets": "QuickWidgets",
}

SOURCE_EXTENSIONS = {".cpp", ".cc", ".cxx", ".c++", ".cp"}
HEADER_EXTENSIONS = {".h", ".hh", ".hpp", ".hxx"}


@dataclass
class ProjectInfo:
    project_dir: pathlib.Path
    name: str
    type: str = "widgets"
    main: str | None = None
    modules: list[str] = field(default_factory=lambda: DEFAULT_WIDGET_MODULES.copy())
    sources: list[str] = field(default_factory=list)
    headers: list[str] = field(default_factory=list)
    forms: list[str] = field(default_factory=list)
    resources: list[str] = field(default_factory=list)
    translations: list[str] = field(default_factory=list)
    include_dirs: list[str] = field(default_factory=list)
    defines: list[str] = field(default_factory=list)
    std: str = "c++2c"
    startup: str = "auto"
    autogen_output_dir: str = ".qtcling/autogen"
    pro_files: list[pathlib.Path] = field(default_factory=list)
    toml_file: pathlib.Path | None = None
    warnings: list[str] = field(default_factory=list)
    unsupported: list[str] = field(default_factory=list)


def unique_preserving_order(values: list[str]) -> list[str]:
    seen: set[str] = set()
    result: list[str] = []
    for value in values:
        if value in seen:
            continue
        seen.add(value)
        result.append(value)
    return result


def strip_comment(line: str) -> str:
    in_quote = False
    result = []
    for ch in line:
        if ch == '"':
            in_quote = not in_quote
        if ch == "#" and not in_quote:
            break
        result.append(ch)
    return "".join(result)


def logical_pro_lines(text: str) -> list[str]:
    lines: list[str] = []
    current = ""
    for raw_line in text.splitlines():
        line = strip_comment(raw_line).rstrip()
        if not line.strip():
            continue
        if line.endswith("\\"):
            current += line[:-1] + " "
            continue
        current += line
        lines.append(current.strip())
        current = ""
    if current.strip():
        lines.append(current.strip())
    return lines


def tokenize_pro_value(value: str) -> list[str]:
    return [token for token in re.split(r"\s+", value.strip()) if token]


def parse_pro_file(path: pathlib.Path) -> dict[str, list[tuple[str, list[str]]]]:
    parsed: dict[str, list[tuple[str, list[str]]]] = {}
    text = path.read_text(encoding="utf-8", errors="ignore")
    for line in logical_pro_lines(text):
        match = re.match(r"([A-Za-z0-9_:.+-]+)\s*([+\-]?=)\s*(.*)$", line)
        if not match:
            continue
        key, operator, value = match.groups()
        key = key.split(":")[-1]
        parsed.setdefault(key, []).append((operator, tokenize_pro_value(value)))
    return parsed


def collect_values(parsed: dict[str, list[tuple[str, list[str]]]], key: str) -> list[str]:
    values: list[str] = []
    for operator, tokens in parsed.get(key, []):
        if operator == "-=":
            values = [value for value in values if value not in tokens]
        else:
            if operator == "=":
                values = []
            values.extend(tokens)
    return unique_preserving_order(values)


def normalize_module(token: str) -> str:
    return MODULE_NAME_MAP.get(token.lower(), token)


def infer_type(configs: list[str], removed_modules: list[str], modules: list[str]) -> str:
    if "cmdline" in configs:
        return "console"
    if any(module.lower() == "gui" for module in removed_modules):
        return "core"
    if "Widgets" in modules:
        return "widgets"
    if "Gui" in modules:
        return "gui"
    return "core"


def infer_std(configs: list[str]) -> str:
    for config in configs:
        if re.fullmatch(r"c\+\+\d+", config):
            return config.replace("++", "++")
    return "c++2c"


def detect_q_object(file_path: pathlib.Path) -> bool:
    try:
        text = file_path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return False
    return "Q_OBJECT" in text


def expected_moc_output(relative_path: str, project_dir: pathlib.Path) -> str:
    path = pathlib.Path(relative_path)
    if path.suffix.lower() in SOURCE_EXTENSIONS:
        return str(path.with_suffix(".moc"))
    return str(path.with_name(f"moc_{path.stem}.cpp"))


def find_single_pro_file(project_dir: pathlib.Path, info: ProjectInfo) -> pathlib.Path | None:
    pro_files = sorted(project_dir.glob("*.pro"))
    info.pro_files = pro_files
    if len(pro_files) > 1:
        info.warnings.append("multiple .pro files found; using the first one")
    return pro_files[0] if pro_files else None


def load_toml_if_available(project_dir: pathlib.Path, info: ProjectInfo) -> None:
    toml_path = project_dir / "qtcling.toml"
    if toml_path.exists():
        info.toml_file = toml_path
        apply_toml(project_dir, toml_path, info)


def coerce_string_list(value: object) -> list[str]:
    if value is None:
        return []
    if isinstance(value, list):
        return [str(item) for item in value]
    if isinstance(value, str):
        return [value]
    return [str(value)]


def apply_toml(project_dir: pathlib.Path, toml_path: pathlib.Path, info: ProjectInfo) -> None:
    if tomllib is None:
        info.warnings.append("qtcling.toml found, but Python tomllib is not available")
        return

    with toml_path.open("rb") as file:
        data = tomllib.load(file)

    project = data.get("project", {})
    autogen = data.get("autogen", {})

    info.name = str(project.get("name", info.name))
    info.type = str(project.get("type", info.type))
    info.main = str(project["main"]) if "main" in project else info.main
    info.startup = str(project.get("startup", info.startup))
    info.sources = coerce_string_list(project.get("sources"))
    info.headers = coerce_string_list(project.get("headers"))
    info.forms = coerce_string_list(project.get("forms"))
    info.resources = coerce_string_list(project.get("resources"))
    info.translations = coerce_string_list(project.get("translations"))
    info.modules = coerce_string_list(project.get("modules")) or info.modules
    info.include_dirs = coerce_string_list(project.get("include_dirs"))
    info.defines = coerce_string_list(project.get("defines"))
    info.std = str(project.get("std", info.std))
    info.autogen_output_dir = str(autogen.get("output_dir", info.autogen_output_dir))


def apply_pro(project_dir: pathlib.Path, pro_path: pathlib.Path, info: ProjectInfo) -> None:
    parsed = parse_pro_file(pro_path)
    qt_added = [normalize_module(value) for value in collect_values(parsed, "QT")]
    qt_removed = [normalize_module(value) for operator, tokens in parsed.get("QT", []) if operator == "-=" for value in tokens]
    configs = collect_values(parsed, "CONFIG")

    if qt_added:
        modules = ["Core"]
        if "Gui" not in qt_removed and "gui" not in [value.lower() for value in qt_removed]:
            modules.append("Gui")
        modules.extend(qt_added)
        info.modules = unique_preserving_order([module for module in modules if module not in qt_removed])
    else:
        info.modules = DEFAULT_WIDGET_MODULES.copy()

    info.type = infer_type(configs, qt_removed, info.modules)
    if info.type in {"console", "core"} and not qt_added:
        info.modules = DEFAULT_CORE_MODULES.copy()

    info.sources = collect_values(parsed, "SOURCES")
    info.headers = collect_values(parsed, "HEADERS")
    info.forms = collect_values(parsed, "FORMS")
    info.resources = collect_values(parsed, "RESOURCES")
    info.translations = collect_values(parsed, "TRANSLATIONS")
    info.include_dirs = collect_values(parsed, "INCLUDEPATH")
    info.defines = collect_values(parsed, "DEFINES")
    info.std = infer_std(configs)

    targets = collect_values(parsed, "TARGET")
    if targets:
        info.name = targets[0]
    if "plugin" in configs:
        info.unsupported.append("Qt plugin project detected (CONFIG += plugin)")
    if any(module.endswith("-private") or module.lower().endswith("-private") for module in qt_added):
        info.unsupported.append("private Qt module detected")


def infer_from_files(project_dir: pathlib.Path, info: ProjectInfo) -> None:
    files = sorted(path for path in project_dir.iterdir() if path.is_file())
    info.sources = [path.name for path in files if path.suffix.lower() in SOURCE_EXTENSIONS and path.name != "startup.cpp"]
    info.headers = [path.name for path in files if path.suffix.lower() in HEADER_EXTENSIONS]
    info.forms = [path.name for path in files if path.suffix.lower() == ".ui"]
    info.resources = [path.name for path in files if path.suffix.lower() == ".qrc"]
    info.translations = [path.name for path in files if path.suffix.lower() == ".ts"]
    info.modules = DEFAULT_WIDGET_MODULES.copy()
    info.type = "widgets" if "main.cpp" in info.sources else "core"


def add_detected_warnings(project_dir: pathlib.Path, info: ProjectInfo) -> None:
    if not info.sources:
        info.warnings.append("no source files detected")
    main_sources = [source for source in info.sources if pathlib.Path(source).name == "main.cpp"]
    if len(main_sources) > 1:
        info.warnings.append("multiple main.cpp files detected")
    if info.main:
        pass
    elif main_sources:
        info.main = main_sources[0]
    elif info.sources:
        info.warnings.append("main.cpp was not detected")

    if list(project_dir.glob("*.qml")) or (project_dir / "qmldir").exists():
        info.unsupported.append("QML files detected")

    for path in list(project_dir.glob("*.cpp")) + list(project_dir.glob("*.h")):
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        if "Q_PLUGIN_METADATA" in text:
            info.unsupported.append("Q_PLUGIN_METADATA detected")
            break


def startup_plan(project_dir: pathlib.Path, info: ProjectInfo) -> list[str]:
    includes: list[str] = []
    includes.extend(source for source in info.sources if pathlib.Path(source).name != "main.cpp")
    for resource in info.resources:
        path = pathlib.Path(resource)
        includes.append(str(path.with_name(f"qrc_{path.stem}.cpp")))
    moc_inputs = info.headers + [source for source in info.sources if pathlib.Path(source).suffix.lower() in SOURCE_EXTENSIONS]
    for relative in moc_inputs:
        full_path = project_dir / relative
        if detect_q_object(full_path):
            includes.append(expected_moc_output(relative, project_dir))
    if info.main:
        includes.append(info.main)
    return unique_preserving_order(includes)


def load_project(project_path: pathlib.Path) -> ProjectInfo:
    project_dir = project_path if project_path.is_dir() else project_path.parent
    project_dir = project_dir.resolve()
    info = ProjectInfo(project_dir=project_dir, name=project_dir.name)
    load_toml_if_available(project_dir, info)
    pro_file = find_single_pro_file(project_dir, info)
    if info.toml_file is None:
        if pro_file:
            apply_pro(project_dir, pro_file, info)
        else:
            infer_from_files(project_dir, info)
    add_detected_warnings(project_dir, info)
    return info


def autogen_outputs(project_dir: pathlib.Path, info: ProjectInfo) -> list[str]:
    outputs: list[str] = []
    for form in info.forms:
        path = pathlib.Path(form)
        outputs.append(str(path.with_name(f"ui_{path.stem}.h")))
    for resource in info.resources:
        path = pathlib.Path(resource)
        outputs.append(str(path.with_name(f"qrc_{path.stem}.cpp")))
    moc_inputs = info.headers + [source for source in info.sources if pathlib.Path(source).suffix.lower() in SOURCE_EXTENSIONS]
    for relative in moc_inputs:
        full_path = project_dir / relative
        if detect_q_object(full_path):
            outputs.append(expected_moc_output(relative, project_dir))
    return unique_preserving_order(outputs)


def print_list(title: str, values: list[str], indent: str = "  ") -> None:
    print(f"{title}:")
    if not values:
        print(f"{indent}(none)")
        return
    for value in values:
        print(f"{indent}{value}")


def explain(project_path: pathlib.Path) -> int:
    if not project_path.exists():
        print(f"qtcling-project: path not found: {project_path}", file=sys.stderr)
        return 1

    info = load_project(project_path)
    print("qtcling-project explain")
    print(f"  project_dir: {info.project_dir}")
    print(f"  qtcling_toml: {info.toml_file if info.toml_file else '(none)'}")
    print(f"  pro_files: {', '.join(str(path.name) for path in info.pro_files) if info.pro_files else '(none)'}")
    print(f"  name: {info.name}")
    print(f"  type: {info.type}")
    print(f"  main: {info.main or '(none)'}")
    print(f"  modules: {' '.join(info.modules) if info.modules else '(none)'}")
    print(f"  std: {info.std}")
    print(f"  startup: {info.startup}")
    print(f"  autogen_output_dir: {info.autogen_output_dir}")
    print()
    print_list("Sources", info.sources)
    print_list("Headers", info.headers)
    print_list("Forms", info.forms)
    print_list("Resources", info.resources)
    print_list("Translations", info.translations)
    print_list("Include dirs", info.include_dirs)
    print_list("Defines", info.defines)
    print_list("Autogen outputs", autogen_outputs(info.project_dir, info))
    print_list("Generated startup includes", startup_plan(info.project_dir, info))
    print_list("Warnings", unique_preserving_order(info.warnings))
    print_list("Unsupported", unique_preserving_order(info.unsupported))
    return 0


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="qtcling-project")
    subparsers = parser.add_subparsers(dest="command", required=True)

    explain_parser = subparsers.add_parser("explain", help="explain qtcling project inputs without generating files")
    explain_parser.add_argument("path", nargs="?", default=".", help="project directory or file")

    args = parser.parse_args(argv)
    if args.command == "explain":
        return explain(pathlib.Path(args.path))
    parser.error(f"unknown command: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
