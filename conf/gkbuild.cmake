# Helper modules.
include(CheckFunctionExists)
include(CheckIncludeFile)

# Setup options.
option(GDB "enable use of GDB" OFF)
option(ASSERT "turn asserts on" OFF)
option(ASSERT2 "additional assertions" OFF)
option(DEBUG "add debugging support" OFF)
option(GPROF "add gprof support" OFF)
option(OPENMP "enable OpenMP support" OFF)
option(PCRE "enable PCRE support" OFF)
option(GKREGEX "enable GKREGEX support" OFF)
option(GKRAND "enable GKRAND support" OFF)

# Add compiler flags.
if(MSVC)
  set(GK_COPTS "/Ox")
  set(GK_COPTIONS "-DWIN32 -DMSC -D_CRT_SECURE_NO_DEPRECATE -DUSE_GKREGEX")
elseif(MINGW)
  set(GK_COPTS "-DUSE_GKREGEX")
else()
  set(GK_COPTIONS "-DLINUX -D_FILE_OFFSET_BITS=64")
endif(MSVC)
if(CYGWIN)
  set(GK_COPTIONS "${GK_COPTIONS} -DCYGWIN")
endif(CYGWIN)
if(CMAKE_COMPILER_IS_GNUCC)
# GCC opts.
  set(GK_COPTIONS "${GK_COPTIONS} -std=c99 -fno-strict-aliasing")
  
  # Handle architecture-specific optimizations
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    # ARM64/Apple Silicon specific optimizations
    set(GK_COPTIONS "${GK_COPTIONS} -mcpu=native")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    # x86_64 specific optimizations
    set(GK_COPTIONS "${GK_COPTIONS} -march=native -mtune=native")
  else()
    # Generic optimizations for other architectures
    set(GK_COPTIONS "${GK_COPTIONS} -mtune=generic")
  endif()
  
  # Enhanced optimization flags for better performance
  set(GK_COPTIONS "${GK_COPTIONS} -ftree-vectorize -ffast-math -funroll-loops")
  set(GK_COPTIONS "${GK_COPTIONS} -fprefetch-loop-arrays -fomit-frame-pointer")
  set(GK_COPTIONS "${GK_COPTIONS} -falign-functions=32 -falign-loops=32")
  if(NOT MINGW)
      set(GK_COPTIONS "${GK_COPTIONS} -fPIC")
  endif(NOT MINGW)
# GCC warnings.
  set(GK_COPTIONS "${GK_COPTIONS} -Werror -Wall -pedantic -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unknown-pragmas")
elseif(${CMAKE_C_COMPILER_ID} MATCHES "Sun")
# Sun insists on -xc99.
  set(GK_COPTIONS "${GK_COPTIONS} -xc99")
endif(CMAKE_COMPILER_IS_GNUCC)

if(${CMAKE_C_COMPILER_ID} STREQUAL "Intel")
  set(GK_COPTIONS "${GK_COPTIONS} -xHost -ipo -no-prec-div -fp-model fast=2")
  set(GK_COPTIONS "${GK_COPTIONS} -qopt-prefetch=3 -qopt-mem-layout-trans=3")
  set(GK_COPTIONS "${GK_COPTIONS} -qopenmp-simd -qopt-report=5")
  #  set(GK_COPTIONS "${GK_COPTIONS} -fast")
endif()

# Add support for the Accelerate framework in OS X
if(APPLE)
  set(GK_COPTIONS "${GK_COPTIONS} -framework Accelerate")
endif(APPLE)

# Find OpenMP if it is requested.
if(OPENMP)
  include(FindOpenMP)
  if(OPENMP_FOUND)
    set(GK_COPTIONS "${GK_COPTIONS} -D__OPENMP__ ${OpenMP_C_FLAGS}")
  else()
    message(WARNING "OpenMP was requested but support was not found")
  endif(OPENMP_FOUND)
endif(OPENMP)


# Add various definitions.
if(GDB)
  set(GK_COPTS "${GK_COPTS} -g")
  set(GK_COPTIONS "${GK_COPTIONS} -Werror")
else()
  set(GK_COPTS "-O3 -flto")
  # Additional aggressive optimizations for release builds
  # Note: -fwhole-program removed as it conflicts with -flto in newer GCC
endif(GDB)


if(DEBUG)
  set(GK_COPTS "-Og")
  set(GK_COPTIONS "${GK_COPTIONS} -DDEBUG")
endif(DEBUG)

if(GPROF)
  set(GK_COPTS "-pg")
endif(GPROF)

if(NOT ASSERT)
  set(GK_COPTIONS "${GK_COPTIONS} -DNDEBUG")
endif(NOT ASSERT)

if(NOT ASSERT2)
  set(GK_COPTIONS "${GK_COPTIONS} -DNDEBUG2")
endif(NOT ASSERT2)


# Add various options
if(PCRE)
  set(GK_COPTIONS "${GK_COPTIONS} -D__WITHPCRE__")
endif(PCRE)

if(GKREGEX)
  set(GK_COPTIONS "${GK_COPTIONS} -DUSE_GKREGEX")
endif(GKREGEX)

if(GKRAND)
  set(GK_COPTIONS "${GK_COPTIONS} -DUSE_GKRAND")
endif(GKRAND)


# Check for features.
check_include_file(execinfo.h HAVE_EXECINFO_H)
if(HAVE_EXECINFO_H)
  set(GK_COPTIONS "${GK_COPTIONS} -DHAVE_EXECINFO_H")
endif(HAVE_EXECINFO_H)

check_function_exists(getline HAVE_GETLINE)
if(HAVE_GETLINE)
  set(GK_COPTIONS "${GK_COPTIONS} -DHAVE_GETLINE")
endif(HAVE_GETLINE)


# Custom check for TLS.
if(MSVC)
  set(GK_COPTIONS "${GK_COPTIONS} -D__thread=__declspec(thread)")

  # This if checks if that value is cached or not.
  if("${HAVE_THREADLOCALSTORAGE}" MATCHES "^${HAVE_THREADLOCALSTORAGE}$")
    try_compile(HAVE_THREADLOCALSTORAGE
      ${CMAKE_BINARY_DIR}
      ${CMAKE_SOURCE_DIR}/conf/check_thread_storage.c)
    if(HAVE_THREADLOCALSTORAGE)
      message(STATUS "checking for thread-local storage - found")
    else()
      message(STATUS "checking for thread-local storage - not found")
    endif()
  endif()
  if(NOT HAVE_THREADLOCALSTORAGE)
    set(GK_COPTIONS "${GK_COPTIONS} -D__thread=")
  endif()
endif()

# Finally set the official C flags.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GK_COPTIONS} ${GK_COPTS}")

