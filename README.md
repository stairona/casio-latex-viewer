# LaTeX Viewer for Casio fx-CG50

A native add-in for the Casio fx-CG50 (Prizm) calculator that renders LaTeX math notation on the 396x224 color display. Browse `.tex` files from calculator storage and view formatted equations.

## Features

- Recursive-descent LaTeX parser (string to AST)
- Two-pass layout engine: measure then draw
- 40+ math symbols via custom 8x10 bitmap glyphs
- File browser for `.tex` files on calculator storage
- Scrollable viewer with color UI

### Supported LaTeX

| Syntax | Example |
|--------|---------|
| Fractions | `\frac{a}{b}` |
| Square roots | `\sqrt{x}` |
| Superscripts | `x^{2}` |
| Subscripts | `a_{n}` |
| Greek letters | `\alpha`, `\beta`, `\omega`, ... |
| Operators | `\sum`, `\int`, `\prod` |
| Relations | `\leq`, `\geq`, `\neq`, `\approx` |
| Arrows | `\to`, `\leftarrow`, `\uparrow`, `\downarrow` |
| Misc | `\pm`, `\times`, `\div`, `\infty` |

## Building

Cross-compiles for SH4 (SuperH) using [gint](https://git.planet-casio.com/Lephenixnoir/gint) inside Docker. No local toolchain needed.

### Prerequisites

- Docker Desktop
- ~2 GB disk space for toolchain image

### Build

```bash
./docker-build.sh
```

Or manually:

```bash
docker build --target export -o dist/ .
```

The multi-stage Docker build:
1. **Stage A** — installs sh-elf-gcc, fxlibc, and gint via [GiteaPC](https://git.planet-casio.com/Lephenixnoir/GiteaPC)
2. **Stage B** — compiles the app with `fxsdk build-cg`
3. **Stage C** — exports `LaTeX_Viewer.g3a` to `dist/`

First build takes ~45 min (toolchain compilation). Rebuilds use cache and take seconds.

## Deploying to Calculator

1. Connect fx-CG50 via USB, select **USB Flash** on the calculator
2. Copy the add-in:
   ```bash
   cp dist/LaTeX_Viewer.g3a /Volumes/CASIO/
   ```
3. Safely eject, then find **LaTeX View** in the Main Menu

Place `.tex` files in the calculator root — the file browser scans `/fls0/` for `.tex` files.

## Controls

**File Browser**: UP/DOWN to navigate, EXE to open, EXIT to quit

**Viewer**: UP/DOWN to scroll, EXIT to return to browser

## Project Structure

```
src/
  main.c            App loop, UI states, input handling
  latex.h/c         Recursive descent LaTeX parser
  render.h/c        AST layout and drawing
  filebrowser.h/c   File listing via gint BFile API
  symbols.h/c       8x10 bitmap glyphs for math symbols
assets/
  icon.png          Application icon
Dockerfile          Multi-stage SH4 cross-compile
docker-build.sh     Build helper script
CMakeLists.txt      CMake config (fxsdk FindGint module)
toolchain.cmake     Standalone SH4 toolchain (host builds)
fxconfig.toml       fxsdk project metadata
```

## Technical Details

- **Target**: Casio fx-CG50 — SH4 (sh3eb-elf), no FPU
- **Display**: 396x224, RGB565
- **HAL**: gint 2.11
- **C stdlib**: fxlibc
- **Compiler**: sh-elf-gcc 14.1.0

## License

MIT
