# LaTeX Viewer for Casio fx-CG50

A native add-in for the Casio fx-CG50 (Prizm) calculator that renders LaTeX math notation on the 396×224 color display. Browse `.tex` files from calculator storage and view formatted equations.

## Features

- Recursive-descent LaTeX parser (string to AST)
- Two-pass layout engine: measure then draw
- 89 math symbols via custom 8×10 bitmap glyphs
- Matrix environments with delimiter rendering
- Accents, function names, and text mode
- File browser for `.tex` files on calculator storage
- Scrollable viewer with color UI

## Supported LaTeX

### Structure

| Syntax | Example |
|--------|---------|
| Fractions | `\frac{a}{b}` |
| Square roots | `\sqrt{x}` |
| Superscripts | `x^{2}` |
| Subscripts | `a_{n}` |
| Grouping | `{...}` |

### Matrix Environments

| Environment | Delimiters |
|-------------|-----------|
| `\begin{pmatrix}...\end{pmatrix}` | Parentheses `( )` |
| `\begin{bmatrix}...\end{bmatrix}` | Brackets `[ ]` |
| `\begin{vmatrix}...\end{vmatrix}` | Pipes `\| \|` |
| `\begin{Bmatrix}...\end{Bmatrix}` | Braces `{ }` |
| `\begin{matrix}...\end{matrix}` | None |
| `\begin{cases}...\end{cases}` | Left brace |
| `\begin{aligned}...\end{aligned}` | None |

Use `&` to separate columns and `\\` to separate rows.

### Accents

`\vec`, `\hat`, `\dot`, `\ddot`, `\bar`, `\tilde`, `\overline`

### Text & Font Commands

| Command | Behavior |
|---------|---------|
| `\text{...}`, `\textbf{...}`, `\mathrm{...}`, `\textrm{...}` | Render content as plain text |
| `\mathbb{X}` | Blackboard bold (double-draw approximation) |
| `\mathbf{...}`, `\mathcal{...}`, `\operatorname{...}` | Pass through to argument |

### Function Names (render as text)

`\det` `\dim` `\ker` `\rank` `\null` `\span`
`\cos` `\sin` `\tan` `\cot` `\sec` `\csc`
`\log` `\ln` `\exp` `\lim` `\min` `\max`
`\inf` `\sup` `\arg` `\deg` `\gcd` `\hom` `\mod` `\Pr` `\tr`

### Symbols

**Lowercase Greek** — `\alpha` `\beta` `\gamma` `\delta` `\epsilon` `\zeta` `\eta` `\theta` `\iota` `\kappa` `\lambda` `\mu` `\nu` `\xi` `\pi` `\rho` `\sigma` `\tau` `\upsilon` `\phi` `\chi` `\psi` `\omega`

**Greek variants** — `\varepsilon` `\vartheta` `\varphi`

**Uppercase Greek** — `\Gamma` `\Delta` `\Theta` `\Lambda` `\Xi` `\Pi` `\Sigma` `\Upsilon` `\Phi` `\Psi` `\Omega`

**Operators** — `\sum` `\prod` `\int` `\iint` `\iiint` `\partial` `\nabla`

**Arithmetic** — `\pm` `\mp` `\times` `\div` `\cdot` `\circ`

**Relations** — `\leq` `\geq` `\neq` `\approx` `\sim` `\cong` `\equiv` `\propto`

**Arrows** — `\to` `\rightarrow` `\leftarrow` `\uparrow` `\downarrow` `\Rightarrow` `\Leftarrow` `\Leftrightarrow` `\mapsto`

**Set theory & logic** — `\in` `\notin` `\subset` `\subseteq` `\supset` `\supseteq` `\cup` `\cap` `\bigcup` `\bigcap` `\emptyset` `\forall` `\exists` `\neg` `\land` `\lor`

**Misc** — `\infty` `\angle` `\degree` `\perp` `\parallel`

### Spacing Commands

`\,` `\;` `\!` `\ ` (all produce spacing adjustments)

### Delimiters

`\left` and `\right` consume their delimiter token and render as parentheses.

---

## Building

Cross-compiles for SH4 (SuperH) using [gint](https://git.planet-casio.com/Lephenixnoir/gint) inside Docker. No local toolchain needed.

See **[DOCKER.md](DOCKER.md)** for the full build guide, including SSD setup and troubleshooting.

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

The multi-stage build:
1. **Stage A** — installs sh-elf-gcc, fxlibc, and gint via [GiteaPC](https://git.planet-casio.com/Lephenixnoir/GiteaPC)
2. **Stage B** — compiles the app with `fxsdk build-cg`
3. **Stage C** — exports `LaTeX_Viewer.g3a` to `dist/`

First build takes ~45 min (toolchain compilation). Rebuilds use cache and take seconds.

---

## Deploying to Calculator

1. Connect fx-CG50 via USB, select **USB Flash** on the calculator
2. Copy the add-in:
   ```bash
   cp dist/LaTeX_Viewer.g3a /Volumes/CASIO/
   ```
3. Copy any `.tex` files you want to view:
   ```bash
   cp myfile.tex /Volumes/CASIO/
   ```
4. Safely eject, then find **LaTeX View** in the Main Menu

The app scans the calculator root (`/fls0/`) for `.tex` files at startup.

---

## Controls

**File Browser**: UP/DOWN to navigate, EXE to open, EXIT to quit

**Viewer**: UP/DOWN to scroll, EXIT to return to browser

---

## Project Structure

```
src/
  main.c            App loop, UI states, input handling
  latex.h/c         Recursive-descent LaTeX parser
  render.h/c        AST layout and drawing
  filebrowser.h/c   File listing via gint BFile API
  symbols.h/c       8x10 bitmap glyphs for 89 math symbols
assets/
  icon.png          Application icon (92x64)
  icon-mono.png     Monotone icon variant
dist/               Build artifacts (gitignored)
Dockerfile          Multi-stage SH4 cross-compile
docker-build.sh     Build helper script
CMakeLists.txt      CMake config (fxsdk FindGint module)
fxconfig.toml       fxsdk project metadata
toolchain.cmake     Standalone SH4 toolchain (host builds)
DOCKER.md           Full Docker build and troubleshooting guide
```

---

## Technical Details

- **Target**: Casio fx-CG50 — SH4 (sh3eb-elf), no FPU
- **Display**: 396×224, RGB565
- **HAL**: gint 2.11
- **C stdlib**: fxlibc
- **Compiler**: sh-elf-gcc 14.1.0

---

## License

MIT
