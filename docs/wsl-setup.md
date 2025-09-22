# Run in WSL 2 + VS Code Remote - WSL

This project contains three services that typically run together:
- `QuantTradeServer` (trade)
- `QuantMarketServer` (market)
- `QuantStrategyClient` (strategy)

All binaries output into each module’s `src/bin` folder and use `./config/config.json` at runtime.

## 1) Prereqs inside WSL
- Ubuntu (WSL2)
- `sudo apt update && sudo apt install -y build-essential cmake gdb git`

Recommended: place the repo under Linux FS for performance, e.g. `~/workspace/quant-trading`.

## 2) Open in VS Code (Remote - WSL)
- Install VS Code extension: “Remote - WSL” and “C/C++”.
- From WSL terminal run: `code ~/workspace/quant-trading`.

## 3) Build
Option A — one command:
- Run VS Code task: `build:all (Release)` (Terminal > Run Task)
  - Or from terminal: `bash shell/build_all.sh`

Option B — per target (presets):
- Strategy: `cmake --build --preset build-release`
- Market: `cmake --build --preset market-build-release`
- Trade: `cmake --build --preset trade-build-release`

Outputs:
- `quant_enterprise_trade/src/bin/QuantTradeServer`
- `quant_enterprise_market/src/bin/QuantMarketServer`
- `quant_enterprise_strategy/src/bin/QuantStrategyClient`

## 4) Configure
Each binary reads `./config/config.json` from its own `src/bin` working directory.
Sample files are already present under each `src/bin/config/`.

## 5) Run
Option A — orchestrated shell:
- `bash shell/start.sh` (spawns trade → market → strategy in background loops)

Option B — debug in VS Code:
- Launch configurations provided:
  - `Launch Trade (WSL)`
  - `Launch Market (WSL)`
  - `Launch Strategy (WSL)`
  - Compound: `Launch All (Trade+Market+Strategy)`

Notes:
- The `shell/run_*.sh` scripts now auto-detect the repo path; no hardcoded user paths.
- If ports/IPs differ from defaults, update each `src/bin/config/config.json` accordingly.

