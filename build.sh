#!/usr/bin/env bash
# macOS/Linux: builds Zeon against SDL3 + shadercross from deps/prefix (see deps/build_deps.sh).
#
# One-time / when deps missing:
#   ./deps/build_deps.sh
# Or this script will run it if libSDL3 is not under deps/prefix yet.
#
# Override: SDL_PREFIX=/path/to/sdl skips deps and uses that prefix (include/, lib/).
#
# Usage:
#   ./build.sh
#   SKIP_SHADERS=1 ./build.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$ROOT/bin"

if [[ -n "${SDL_PREFIX:-}" ]]; then
  SDL_INC="$SDL_PREFIX/include"
  SDL_LIB="$SDL_PREFIX/lib"
  if [[ ! -d "$SDL_INC/SDL3" ]]; then
    echo "SDL_PREFIX=$SDL_PREFIX has no include/SDL3" >&2
    exit 1
  fi
else
  DEPS_PREFIX="$ROOT/deps/prefix"
  if [[ ! -f "$DEPS_PREFIX/lib/libSDL3.dylib" && ! -f "$DEPS_PREFIX/lib/libSDL3.so" ]]; then
    echo "SDL3 not found under deps/prefix; running deps/build_deps.sh (clone + cmake install)..."
    "$ROOT/deps/build_deps.sh"
  fi
  SDL_INC="$DEPS_PREFIX/include"
  SDL_LIB="$DEPS_PREFIX/lib"
fi

if [[ "${SKIP_SHADERS:-}" != "1" ]]; then
  "$ROOT/shaders.sh"
else
  echo "Skipping shaders.sh (SKIP_SHADERS=1)"
fi

/usr/bin/clang++ \
  -std=c++20 -g \
  -I"$ROOT/vendor/cgltf" \
  -I"$ROOT/vendor/stb" \
  "-I$SDL_INC" \
  "$ROOT/src/main.cpp" \
  -o "$ROOT/bin/main" \
  "-L$SDL_LIB" \
  -lSDL3 \
  "-Wl,-rpath,$SDL_LIB"

echo "Built $ROOT/bin/main"
