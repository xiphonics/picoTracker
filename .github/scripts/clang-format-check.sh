#!/usr/bin/env bash

set -euo pipefail

CLANG_FORMAT_BIN="${CLANG_FORMAT_BIN:-clang-format-17}"

if ! command -v "${CLANG_FORMAT_BIN}" >/dev/null 2>&1; then
  echo "error: ${CLANG_FORMAT_BIN} not found in PATH"
  exit 1
fi

mapfile -d '' files < <(
  find sources -type f \
    ! -path 'sources/Externals/*' \
    \( \
      -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o \
      -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' -o \
      -name '*.proto' \
    \) \
    -print0
)

if [ "${#files[@]}" -eq 0 ]; then
  echo "No matching source files found under sources/"
  exit 0
fi

echo "Checking ${#files[@]} files with ${CLANG_FORMAT_BIN}..."
"${CLANG_FORMAT_BIN}" \
  --style=file \
  --fallback-style=llvm \
  --dry-run \
  --Werror \
  "${files[@]}"
