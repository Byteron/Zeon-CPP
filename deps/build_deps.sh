#!/usr/bin/env bash
# Clone (if missing), build, and install SDL3 + SDL_shadercross into deps/prefix.
# Unified layout: include/, lib/, bin/ — same relative paths on macOS and Linux.
#
# Requires: git, cmake, a C compiler, and ninja (recommended) or default CMake generator.
# Optional: ZEON_SDL_REF=release-3.2.x  shallow-clone that SDL branch/tag.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PREFIX="$ROOT/deps/prefix"
BUILD="$ROOT/deps/build"
SDL_SRC="$ROOT/deps/SDL"
SC_SRC="$ROOT/deps/SDL_shadercross"

mkdir -p "$BUILD"

clone_if_needed() {
  if [[ ! -d "$SDL_SRC/.git" ]]; then
    echo "Cloning SDL into $SDL_SRC ..."
    if [[ -n "${ZEON_SDL_REF:-}" ]]; then
      git clone --depth 1 --branch "$ZEON_SDL_REF" https://github.com/libsdl-org/SDL.git "$SDL_SRC"
    else
      git clone --depth 1 https://github.com/libsdl-org/SDL.git "$SDL_SRC"
    fi
  fi

  if [[ ! -d "$SC_SRC/.git" ]]; then
    echo "Cloning SDL_shadercross into $SC_SRC (submodules) ..."
    git clone --recursive https://github.com/libsdl-org/SDL_shadercross.git "$SC_SRC"
  fi
}

cmake_sdl() {
  local gen=()
  if command -v ninja >/dev/null 2>&1; then
    gen=(-G Ninja)
  fi
  cmake "${gen[@]}" -S "$SDL_SRC" -B "$BUILD/SDL" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DSDL_SHARED=ON \
    -DSDL_STATIC=OFF
  cmake --build "$BUILD/SDL" --parallel "$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"
  cmake --install "$BUILD/SDL"
}

cmake_shadercross() {
  if [[ -f "$BUILD/SDL_shadercross/CMakeCache.txt" ]] && ! grep -q 'SDLSHADERCROSS_VENDORED:BOOL=ON' "$BUILD/SDL_shadercross/CMakeCache.txt" 2>/dev/null; then
    echo "Removing SDL_shadercross build dir (reconfigure with SDLSHADERCROSS_VENDORED=ON)..."
    rm -rf "$BUILD/SDL_shadercross"
  fi

  local gen=()
  if command -v ninja >/dev/null 2>&1; then
    gen=(-G Ninja)
  fi
  # Vendored: build SPIRV-Cross / SPIRV-Tools from external/ (avoids system spirv_cross_c_shared).
  # Install: puts shadercross + libs under PREFIX. DXC=ON builds DirectXShaderCompiler (slow on macOS).
  cmake "${gen[@]}" -S "$SC_SRC" -B "$BUILD/SDL_shadercross" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DSDLSHADERCROSS_VENDORED=ON \
    -DSDLSHADERCROSS_INSTALL=ON \
    -DCMAKE_INSTALL_RPATH='@loader_path/../lib' \
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON \
    -DCMAKE_MACOSX_RPATH=ON
  cmake --build "$BUILD/SDL_shadercross" --parallel "$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"
  cmake --install "$BUILD/SDL_shadercross" || true
}

ensure_shadercross_cli() {
  if [[ -x "$PREFIX/bin/shadercross" ]]; then
    return
  fi
  local cand
  for cand in "$BUILD/SDL_shadercross/shadercross" "$BUILD/SDL_shadercross/Release/shadercross"; do
    if [[ -x "$cand" ]]; then
      mkdir -p "$PREFIX/bin"
      cp "$cand" "$PREFIX/bin/shadercross"
      chmod +x "$PREFIX/bin/shadercross"
      echo "Installed shadercross into $PREFIX/bin/"
      return
    fi
  done
  echo "warning: shadercross not found under $PREFIX/bin after install; check SDL_shadercross build output" >&2
}

clone_if_needed

if [[ -d "$SC_SRC/.git" ]]; then
  echo "Updating SDL_shadercross submodules..."
  git -C "$SC_SRC" submodule update --init --recursive
fi

cmake_sdl
cmake_shadercross
ensure_shadercross_cli

echo "deps ready at $PREFIX"
