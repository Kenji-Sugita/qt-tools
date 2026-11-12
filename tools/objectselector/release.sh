#!/bin/sh

set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
version_file="$project_root/VERSION"

if [ ! -f "$version_file" ]; then
    echo "VERSION file not found: $version_file" >&2
    exit 1
fi

version=$(tr -d '[:space:]' < "$version_file")
case "$version" in
    *[!0-9.]* | *.*.*.* | .* | *.)
        echo "VERSION must contain a semantic version (major.minor.patch)." >&2
        exit 1
        ;;
esac


if ! printf '%s\n' "$version" | awk -F. 'NF == 3 && $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ && $3 ~ /^[0-9]+$/ { valid = 1 } END { exit !valid }'; then
    echo "VERSION must contain a semantic version (major.minor.patch)." >&2
    exit 1
fi

release_name="objectselector-$version"
release_dir="$project_root/release"
archive="$release_dir/$release_name-source.zip"
staging_dir=$(mktemp -d "${TMPDIR:-/tmp}/objectselector-release.XXXXXX")

cleanup()
{
    rm -rf "$staging_dir"
}

trap cleanup EXIT HUP INT TERM

mkdir -p "$release_dir" "$staging_dir/$release_name"

for path in CMakeLists.txt VERSION USER_GUIDE.md release.sh src examples tests; do
    if [ ! -e "$project_root/$path" ]; then
        echo "Release input not found: $path" >&2
        exit 1
    fi
    cp -R "$project_root/$path" "$staging_dir/$release_name/"
done

rm -f "$archive"
(
    cd "$staging_dir"
    zip -qry "$archive" "$release_name"
)

unzip -t "$archive" >/dev/null
shasum -a 256 "$archive"
printf 'Created %s\n' "$archive"
