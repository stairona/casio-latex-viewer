# SSD-First Docker Implementation Plan

## Goal
Run the full Docker implementation for LaTeX Viewer with Docker Desktop data, images, and build cache on the external SSD at /Volumes/SSD so internal Mac storage does not balloon.

## Scope
- Included: local Docker setup + local Docker build/export flow for LaTeX_Viewer.g3a
- Excluded (for now): GitHub Actions CI workflow
- Deployment: USB copy to calculator remains host-side (outside Docker)

## Decisions Locked
1. Docker engine: Docker Desktop
2. Storage strategy: full Docker runtime/images/cache on SSD
3. SSD mount path: /Volumes/SSD
4. Build model: multi-stage Docker build

## Status (2026-04-16)
| Phase | Status |
|-------|--------|
| 0 — SSD + Docker setup | ✅ Complete |
| 1 — Script hardening | ✅ Complete |
| 2 — toolchain.cmake | ✅ Complete (kept for host builds; Docker uses fxsdk toolchain) |
| 3 — .gitignore + .dockerignore | ✅ Complete |
| 4 — Dockerfile | ✅ Complete — produces LaTeX_Viewer.g3a (78K) |
| 5 — DOCKER.md | ⏳ Pending |
| 6 — Verify + deploy | ✅ Verified on emulator — awaiting physical calculator test |

## Completed (last session)
- Pushed all commits to GitHub (remote up to date at 5d550c8)
- Analyzed `mideterm2.pdf` (Math 232 Midterm 2) — identified all missing features for exam cheat sheet
- Gap analysis: matrix environments, function names, `\text{}`, `\mathbb{R}`, `\vec`, `\left/\right` needed

## Current Task → ✅ Push to GitHub + gap analysis complete

## Next Task
- Implement all gap features for midterm cheat sheet:
  1. Function-name text fallthrough (`\det`, `\ker`, `\dim`, `\text{...}`) — ~30 min
  2. `\mathbb{R}` bitmap glyph — ~15 min
  3. Matrix environments (`\begin{pmatrix}`, `\begin{bmatrix}`, `\begin{vmatrix}`) with `&`/`\\` parsing — ~2-4 hrs
  4. `\vec{v}` accent rendering — ~30 min
  5. `\left( \right)` auto-sizing delimiters — ~1 hr
- Create `midterm2_cheatsheet.tex` with all exam formulas + examples
- Docker build + test on physical fx-CG50
- Write DOCKER.md (Phase 5)

## Open Risks
- Large .tex files may exceed 512K RAM
- Matrix environment rendering requires new AST node type + 2D grid layout in renderer
- `\mathbb` glyphs (blackboard bold R, N, Z, Q, C) need special font-style rendering, not just bitmap lookup
- Exam deadline pressure — prioritize features that unblock the cheat sheet

---

## Phase 0: SSD Docker Foundation (Blocking)
1. Verify /Volumes/SSD is mounted and stable.
2. Ensure SSD has healthy free space (recommended 120 GB+).
3. Create SSD Docker directories:
   - /Volumes/SSD/docker-data
   - /Volumes/SSD/docker-cache
4. In Docker Desktop, move Docker disk image/data location to SSD path.
5. Restart Docker Desktop and validate relocation with:
   - docker info
   - a test image pull/build
6. Confirm growth occurs on SSD path, not internal disk.

## Phase 1: Build Script Hardening
1. Update BUILD_AND_DEPLOY.sh:
   - remove hard-coded absolute project path
   - derive project root from script location
   - add NO_PROMPTS mode
   - add SKIP_DEPLOY mode
   - add artifact validation (exists, non-zero, ELF check)
2. Update setup_phase1.sh:
   - keep PATH persistence opt-in
   - avoid mandatory shell profile mutation
   - keep idempotent behavior

## Phase 2: Deterministic CMake Cross-Compile
1. Add toolchain.cmake with SH4 compiler defaults and flags.
2. Update CMakeLists.txt to support consistent toolchain-file builds.
3. Align host and Docker CMake invocation to avoid drift.

## Phase 3: Repo Hygiene for Docker
1. Add .gitignore for:
   - build artifacts
   - CMake cache/intermediates
   - local toolchain outputs
   - macOS junk files
2. Add .dockerignore to reduce context and SSD churn.

## Phase 4: Docker Implementation
1. Add multi-stage Dockerfile:
   - Stage A: toolchain prerequisites + SH4 toolchain layer
   - Stage B: app build using CMake + toolchain.cmake
   - Stage C: artifact-only export stage with LaTeX_Viewer.g3a
2. Add build helper script to standardize:
   - image build
   - artifact export to dist/
3. Enforce build-only behavior in Docker (no USB deploy logic).

## Phase 5: Documentation
1. Update README:
   - SSD-first Docker setup
   - host vs Docker build paths
2. Add DOCKER.md:
   - Docker Desktop SSD configuration
   - build/export commands
   - troubleshooting (SSD unplugged, mount rename, cache issues)

## Phase 6: Verification and Guardrails
1. Baseline internal disk and SSD usage.
2. Run Docker build and export artifact.
3. Verify artifact:
   - file exists in dist/
   - non-zero size
   - valid ELF metadata
4. Confirm no deploy attempt in Docker path.
5. Run optional host deploy regression only when explicitly enabled.

## Deliverables
- Hardened scripts for automation
- toolchain.cmake for deterministic cross-compilation
- .gitignore and .dockerignore
- Multi-stage Dockerfile
- Docker build helper script
- Updated README and DOCKER.md

## Success Criteria
1. Docker image layers/caches are written to /Volumes/SSD.
2. Internal storage growth from Docker activity is negligible.
3. LaTeX_Viewer.g3a is reproducibly built via Docker and exported to host dist/.
4. USB deploy flow remains host-side and unaffected.
