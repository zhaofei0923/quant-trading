#!/usr/bin/env bash
set -euo pipefail

# Build all components (Release) inside WSL/Unix
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "[Build] Using root: $ROOT_DIR"

build_one() {
  local name="$1"; shift
  local src_rel="$1"; shift
  local build_dir="$ROOT_DIR/$src_rel/../build"
  local src_dir="$ROOT_DIR/$src_rel"
  echo "[Build] $name -> $build_dir"
  cmake -S "$src_dir" -B "$build_dir" -D CMAKE_BUILD_TYPE=Release
  cmake --build "$build_dir" -j
}

build_one "QuantTradeServer" "quant_enterprise_trade/src"
build_one "QuantMarketServer" "quant_enterprise_market/src"
build_one "QuantStrategyClient" "quant_enterprise_strategy/src"

echo "[Build] Done. Binaries in each src/bin folder."

