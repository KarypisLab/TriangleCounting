cmake_minimum_required(VERSION 2.8)
project(TC C)

set(SHARED FALSE CACHE BOOL "build a shared library")

if(MSVC)
  set(TC_INSTALL FALSE)
else()
  set(TC_INSTALL TRUE)
endif()

# Add GK's standard cmake settings
include(./conf/gkbuild.cmake)

# Source files
file(GLOB tc_sources *.c)

# Include directories
include_directories(.)
include_directories(~/local/include)
if(GKLIB_PATH)
  include_directories(${GKLIB_PATH}/include)
endif(GKLIB_PATH)

# Link directories 
link_directories(~/local/lib)
if(GKLIB_PATH)
  link_directories(${GKLIB_PATH}/lib)
endif(GKLIB_PATH)

# Build
add_executable(${PKGNAME} ${tc_sources})

foreach(prog ${PKGNAME})
  target_link_libraries(${prog} GKlib m)
endforeach(prog)

# Install
if(TC_INSTALL)
  install(TARGETS ${PKGNAME} 
    RUNTIME DESTINATION bin)
endif()
