#!/usr/bin/env python3
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parent
PROJECTS = ROOT / "projects"
BUILDS = ROOT / "builds"
LOGS = ROOT / "logs"
QT_PREFIX = Path("/usr/local/qt/Qt/6.10.2/macos")


def run(cmd, log_path):
    proc = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    log_path.write_text(proc.stdout, encoding="utf-8")
    return proc.returncode


def main():
    if not PROJECTS.exists():
        print(f"missing projects directory: {PROJECTS}", file=sys.stderr)
        return 2

    if BUILDS.exists():
        shutil.rmtree(BUILDS)
    if LOGS.exists():
        shutil.rmtree(LOGS)
    BUILDS.mkdir(parents=True)
    LOGS.mkdir(parents=True)

    projects = sorted(path for path in PROJECTS.iterdir() if path.is_dir())
    results = []

    for project in projects:
        build_dir = BUILDS / project.name
        build_dir.mkdir(parents=True)

        configure_log = LOGS / f"{project.name}.configure.log"
        build_log = LOGS / f"{project.name}.build.log"

        configure_cmd = [
            "cmake",
            "-S",
            str(project),
            "-B",
            str(build_dir),
            "-G",
            "Ninja",
            f"-DCMAKE_PREFIX_PATH={QT_PREFIX}",
            "-DCMAKE_BUILD_TYPE=Release",
        ]
        configure_rc = run(configure_cmd, configure_log)

        build_rc = None
        if configure_rc == 0:
            build_cmd = [
                "cmake",
                "--build",
                str(build_dir),
                "--parallel",
            ]
            build_rc = run(build_cmd, build_log)

        status = "PASS" if configure_rc == 0 and build_rc == 0 else "FAIL"
        results.append((project.name, status, configure_rc, build_rc))
        print(f"{status} {project.name}")

    report_lines = [
        "# Validation Report",
        "",
        f"Qt prefix: `{QT_PREFIX}`",
        "Generator: `Ninja`",
        "",
        "| Pattern project | Result | Configure | Build |",
        "| --- | --- | ---: | ---: |",
    ]
    for name, status, configure_rc, build_rc in results:
        build_text = "" if build_rc is None else str(build_rc)
        report_lines.append(f"| `{name}` | {status} | {configure_rc} | {build_text} |")

    report = ROOT / "validation_report.md"
    report.write_text("\n".join(report_lines) + "\n", encoding="utf-8")

    failures = [item for item in results if item[1] != "PASS"]
    print(f"\nReport: {report}")
    print(f"Passed: {len(results) - len(failures)}/{len(results)}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
