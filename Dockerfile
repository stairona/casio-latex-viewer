# Dockerfile — LaTeX Viewer for Casio fx-CG50
#
# Multi-stage build:
#   Stage A (toolchain): installs sh-elf-gcc + fxlibc + gint via GiteaPC
#                        (lephenixnoir's official package manager for the Casio ecosystem)
#   Stage B (build):     compiles the app via fxsdk build-cg (official build system)
#   Stage C (export):    holds only the final .g3a artifact
#
# Usage:  ./docker-build.sh
# Manual: docker build --target export -o dist/ .

# ── Stage A: Toolchain via GiteaPC ───────────────────────────────────────────
FROM debian:12 AS toolchain

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake curl flex git libglib2.0-dev libgmp-dev \
    libmpc-dev libmpfr-dev libncurses-dev libpng-dev libppl-dev \
    libsdl2-dev libusb-1.0-0-dev \
    pkg-config python3 python3-pil python3-venv texinfo \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /casio

# Bootstrap GiteaPC (lephenixnoir's package manager for the Casio ecosystem)
RUN git clone --depth=1 https://git.planet-casio.com/Lephenixnoir/GiteaPC /casio/giteapc \
    && cd /casio/giteapc \
    && python3 giteapc.py install -y Lephenixnoir/GiteaPC

ENV PATH="$PATH:/root/.local/bin"

# Install cross-compiler + C stdlib + gint HAL.
# fxsdk:noudisks2 must be listed explicitly so GiteaPC uses that variant
# (the default fxsdk requires libusb-1.0 which is not needed inside Docker).
RUN giteapc install -y Lephenixnoir/fxsdk:noudisks2 Lephenixnoir/sh-elf-binutils Lephenixnoir/sh-elf-gcc
RUN giteapc install -y Vhex-Kernel-Core/fxlibc
RUN giteapc install -y Lephenixnoir/gint

# Verify toolchain is usable
RUN sh-elf-gcc --version

# ── Stage B: App build via fxsdk ─────────────────────────────────────────────
FROM toolchain AS build

WORKDIR /src
COPY src/            ./src/
COPY assets/         ./assets/
COPY CMakeLists.txt  ./
COPY fxconfig.toml   ./

# fxsdk build-cg: configures CMake with the correct toolchain, linker script,
# FindGint module, and library link order, then builds the project.
RUN fxsdk build-cg

# fxgxa converts the raw binary to a .g3a add-in file with Casio header
RUN cd build-cg \
    && fxgxa --g3a -n "LaTeX View" LaTeX_Viewer.bin -o LaTeX_Viewer.g3a \
    && ls -lh LaTeX_Viewer.g3a

# ── Stage C: Export artifact only ────────────────────────────────────────────
FROM scratch AS export

COPY --from=build /src/build-cg/LaTeX_Viewer.g3a /LaTeX_Viewer.g3a
