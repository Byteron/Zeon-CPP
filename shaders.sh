#!/usr/bin/env bash
# Cross-compile default HLSL sources to MSL for macOS Metal (SDL GPU).
# Uses shadercross from deps/prefix/bin when present, else PATH / SHADERCROSS.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$ROOT/bin/shaders"

find_shadercross() {
  local bundled="$ROOT/deps/prefix/bin/shadercross"
  if [[ -x "$bundled" ]]; then
    echo "$bundled"
    return
  fi
  if [[ -n "${SHADERCROSS:-}" ]]; then
    if [[ -x "$SHADERCROSS" ]]; then
      echo "$SHADERCROSS"
      return
    fi
    if command -v "$SHADERCROSS" >/dev/null 2>&1; then
      command -v "$SHADERCROSS"
      return
    fi
  fi
  if command -v shadercross >/dev/null 2>&1; then
    command -v shadercross
    return
  fi
  echo "shadercross not found. Run ./deps/build_deps.sh or set SHADERCROSS=..." >&2
  exit 1
}

SHADERCROSS="$(find_shadercross)"

# Bundled shadercross links libSDL3_shadercross (and vendored SPIRV libs) with @rpath → ../lib.
# Until reinstall with CMAKE_INSTALL_RPATH, help the loader (also covers Linux).
DEPS_LIB="$ROOT/deps/prefix/lib"
if [[ "$SHADERCROSS" == "$ROOT/deps/prefix/"* ]]; then
  case "$(uname -s)" in
    Darwin)
      export DYLD_FALLBACK_LIBRARY_PATH="$DEPS_LIB${DYLD_FALLBACK_LIBRARY_PATH:+:$DYLD_FALLBACK_LIBRARY_PATH}"
      ;;
    Linux)
      export LD_LIBRARY_PATH="$DEPS_LIB${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
      ;;
  esac
fi

VERT="$ROOT/assets/shaders/solid_skinned.vert.hlsl"
FRAG="$ROOT/assets/shaders/solid_skinned.frag.hlsl"

"$SHADERCROSS" "$VERT" -s HLSL -d MSL -t vertex -e main -o "$ROOT/bin/shaders/solid_skinned.vert.msl"
"$SHADERCROSS" "$FRAG" -s HLSL -d MSL -t fragment -e main -o "$ROOT/bin/shaders/solid_skinned.frag.msl"
