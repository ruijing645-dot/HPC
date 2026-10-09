#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${BUILD_DIR:-"${script_dir}/build"}"

cmake -S "${script_dir}" -B "${build_dir}" "$@"
cmake --build "${build_dir}" --parallel
ctest --test-dir "${build_dir}" --output-on-failure
