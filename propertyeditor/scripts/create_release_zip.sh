#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
project_name="$(basename "${repo_root}")"
version="$(tr -d '[:space:]' < "${repo_root}/VERSION")"
release_dir="${repo_root}/release"
release_name="${project_name}-${version}"
staging_dir="$(mktemp -d)"
archive_path="${release_dir}/${release_name}-source.zip"

cleanup() {
    rm -rf "${staging_dir}"
}
trap cleanup EXIT

mkdir -p "${staging_dir}/${release_name}"
mkdir -p "${release_dir}"

files=(
    CMakeLists.txt
    VERSION
    USER_GUIDE.md
)

dirs=(
    src
    examples
    tests
)

for file in "${files[@]}"; do
    if [[ -f "${repo_root}/${file}" ]]; then
        cp "${repo_root}/${file}" "${staging_dir}/${release_name}/"
    fi
done

for dir in "${dirs[@]}"; do
    if [[ -d "${repo_root}/${dir}" ]]; then
        cp -R "${repo_root}/${dir}" "${staging_dir}/${release_name}/"
    fi
done

(
    cd "${staging_dir}"
    rm -f "${archive_path}"
    zip -qr "${archive_path}" "${release_name}"
)

printf '%s\n' "${archive_path}"
