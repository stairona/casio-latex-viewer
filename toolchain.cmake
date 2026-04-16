# toolchain.cmake — SH4 bare-metal cross-compile for Casio fx-CG50
#
# Usage:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
#
# The compiler is expected to be on PATH as sh-elf-gcc / sh-elf-g++.
# Inside Docker, the toolchain is built by Stage A and placed on PATH.
# On the host (fallback), install via: brew install crosstool-ng && ct-ng sh4 && ct-ng build

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR sh4)

# Compiler binaries — must be on PATH
set(CMAKE_C_COMPILER   sh-elf-gcc)
set(CMAKE_CXX_COMPILER sh-elf-g++)
set(CMAKE_OBJCOPY      sh-elf-objcopy)
set(CMAKE_SIZE         sh-elf-size)

# SH4 target flags for Casio fx-CG50
#   -m4-nofpu    : SH4 without FPU (matches GiteaPC's --with-multilib-list=m3,m4-nofpu)
#   -mb          : big-endian (SH target is sh3eb-elf)
#   -mhitachi    : Hitachi calling convention (required by Casio OS)
#   -Os          : optimize for size (calculator RAM is limited)
# Note: do NOT use -nostdlib here — fxlibc provides the C stdlib for this target
set(SH4_FLAGS "-m4-nofpu -mb -mhitachi -Os -DFXCG50 -fno-lto")

set(CMAKE_C_FLAGS_INIT   "${SH4_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${SH4_FLAGS} -fno-exceptions -fno-rtti")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${SH4_FLAGS}")

# Prevent CMake from trying to link/run test executables on the host
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Do not search host paths for headers or libraries
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
