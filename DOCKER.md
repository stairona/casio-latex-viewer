# DOCKER.md — LaTeX Viewer for Casio fx-CG50

Build guide for the Docker-based SH4 cross-compile workflow.

---

## Overview

The build runs entirely inside Docker — no local toolchain needed. A multi-stage Dockerfile:

| Stage | What it does |
|-------|-------------|
| **A — toolchain** | Installs sh-elf-gcc, fxlibc, and gint via GiteaPC (45–60 min, first time only) |
| **B — build** | Compiles the app with `fxsdk build-cg` |
| **C — export** | Copies only `LaTeX_Viewer.g3a` to the host `dist/` folder |

Output: `dist/LaTeX_Viewer.g3a`

---

## Prerequisites

- **Docker Desktop** installed and running
- **~2 GB free disk space** for the toolchain image (see SSD setup below)
- macOS (tested on M4, Apple Silicon)

---

## SSD Setup (Recommended)

The toolchain image is ~2 GB. To keep it off internal storage, point Docker Desktop to an external SSD.

1. Mount your SSD — verify it is at `/Volumes/SSD`
2. Create the Docker data directory:
   ```bash
   mkdir -p /Volumes/SSD/docker-data
   ```
3. Open **Docker Desktop → Settings → Resources → Advanced**
4. Set **Disk image location** to `/Volumes/SSD/docker-data`
5. Click **Apply & Restart**
6. Verify:
   ```bash
   docker info | grep "Docker Root Dir"
   # Should show: /Volumes/SSD/docker-data
   ```

> If the SSD is not mounted when you run a build, `docker-build.sh` will warn you and ask whether to continue.

---

## Building

### Standard build

```bash
./docker-build.sh
```

This script:
1. Checks Docker is running
2. Warns if SSD is not mounted
3. Runs `docker build --target export --output dist/ .`
4. Validates the artifact (exists, non-zero, valid ELF magic)

Output: `dist/LaTeX_Viewer.g3a`

### Force full rebuild (no cache)

```bash
./docker-build.sh --no-cache
```

Use this if you change the Dockerfile or suspect a corrupt layer.

### Manual build (without the script)

```bash
docker build --target export -o dist/ .
```

---

## Build Times

| Scenario | Time |
|----------|------|
| First build (toolchain compile from source) | 45–60 min |
| Rebuild after source code change | ~10 sec |
| Rebuild after `--no-cache` | 45–60 min |

Layer A (toolchain) is cached after the first build. Only Stage B re-runs when source files change.

---

## Deploying to Calculator

1. Connect the fx-CG50 via USB and select **USB Flash** on the calculator
2. Copy the add-in:
   ```bash
   cp dist/LaTeX_Viewer.g3a /Volumes/CASIO/
   ```
3. Safely eject
4. Find **LaTeX View** in the Main Menu

To also copy `.tex` files:
```bash
cp dist/midterm2_cheatsheet.tex /Volumes/CASIO/
```

The app scans `/fls0/` (calculator root) for `.tex` files at startup.

---

## Troubleshooting

### Docker is not running
```
ERROR: Docker is not running. Start Docker Desktop and try again.
```
Open Docker Desktop and wait for the engine to start (whale icon in menu bar stops animating).

---

### SSD not mounted
```
WARNING: /Volumes/SSD/docker-data not found.
```
Plug in the SSD and verify it mounts at `/Volumes/SSD`. If it mounts under a different name, update the path in `docker-build.sh` (line: `if [ ! -d "/Volumes/SSD/docker-data" ]`).

---

### Artifact not found after build
```
ERROR: Artifact not found at dist/LaTeX_Viewer.g3a
```
The `fxgxa` step in Stage B may have failed silently. Run with full output:
```bash
docker build --target export -o dist/ . --progress=plain 2>&1 | tail -50
```
Look for errors in the `fxgxa` or `fxsdk build-cg` lines.

---

### ELF magic check fails
```
ERROR: Artifact is not a valid ELF binary (magic: ...)
```
The `.g3a` file is not a raw ELF — this is expected. The check in `docker-build.sh` verifies the inner ELF embedded by `fxgxa`. If this fails, the `fxgxa` conversion likely produced a truncated or corrupt file. Re-run with `--no-cache`.

---

### GiteaPC install hangs or fails (network error during first build)
GiteaPC clones from `git.planet-casio.com`. If your network is slow or the server is down, the build will fail during Stage A. Re-run after checking connectivity:
```bash
curl -I https://git.planet-casio.com
```

---

### Docker using internal storage despite SSD setup
Verify the disk image location after restarting Docker Desktop:
```bash
docker info | grep "Docker Root Dir"
```
If it still points to `~/.docker` or `~/Library/Containers/...`, the setting may not have applied. Try moving the location again and doing a full restart.

---

## File Reference

| File | Purpose |
|------|---------|
| `Dockerfile` | Multi-stage build definition |
| `docker-build.sh` | Build helper with validation and SSD check |
| `CMakeLists.txt` | CMake config using `find_package(Gint)` + `GenerateG3A` |
| `fxconfig.toml` | fxsdk project metadata (name, icons) |
| `toolchain.cmake` | Standalone SH4 toolchain file (for host builds, not used by Docker) |
| `dist/LaTeX_Viewer.g3a` | Build artifact (gitignored) |
