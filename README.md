# TriangleCounting
OpenMP-based parallel program for counting the number of triangles in a sparse graph.


## Build requirements
 - CMake 2.8, found at http://www.cmake.org/, as well as GNU make. 
 - Download, build, and install [GKlib](https://github.com/KarypisLab/GKlib).

Assumming that the above are available, two commands should suffice to 
build the software:

     $ make config 
     $ make


## Configuring the build
It is primarily configured by passing options to make config. For example:

     $ make config cc=icc

would configure it to be built using icc.

Configuration options are:
  cc=[compiler]     - The C compiler to use [default: gcc]
  prefix=[PATH]     - Set the installation prefix [default: ~/local]
  gklib_path=[PATH] - Where GKlib was installed [default: ~/local]


## Building and installing
To build and install, run

    $ make install

## Other make commands
   $ make uninstall 
          Removes all files installed by 'make install'.
   
   $ make clean 
          Removes all object files but retains the configuration options.
   
   $ make distclean 
          Performs clean and completely removes the build directory.

## Contact information
George Karypis, karypis@umn.edu
