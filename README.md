# TriangleCounting
OpenMP-based parallel program for counting the number of triangles in a sparse graph.


## Build requirements
 - CMake 2.8, found at http://www.cmake.org/, as well as GNU make. 
 - Download, build, and install [GKlib](https://github.com/KarypisLab/GKlib).

Assumming that the above are available, two commands should suffice to 
build the software:
```
make config 
make
```

## Configuring the build
It is primarily configured by passing options to make config. For example:
```
make config cc=icc
```

would configure it to be built using icc.

Configuration options are:
```
cc=[compiler]     - The C compiler to use [default: gcc]
prefix=[PATH]     - Set the installation prefix [default: ~/local]
gklib_path=[PATH] - Where GKlib was installed [default: ~/local]
```


## Building and installing
To build and install, run the following
```
make
make install
```

By default, the binary, called _gktc_, will be installed in ~/local/bin.
For usage information just type
```
gktc -help
```

## Other make commands
    make uninstall 
         Removes all files installed by 'make install'.
   
    make clean 
         Removes all object files but retains the configuration options.
   
    make distclean 
         Performs clean and completely removes the build directory.


## Performance 
The following shows a sample of gktc's performance on Intel's KnightsLanding
processor:

```
rmat scale25 
----------------------------------------
#p     total    ppt      tct   speedup 
1      767.4  141.9    625.5   
5      154.4   29.1    125.2     5.0x 
10      77.2   14.5     62.6     9.9x
20      38.6    7.3     31.2    19.9x
40      19.6    3.7     16.1    39.2x
68      12.2    2.2      9.9    62.9x
136      9.3    1.7      7.6    82.5x
272     10.1    1.5      8.6    76.0x

twitter 
----------------------------------------
#p     total    ppt      tct   speedup    
1     1422.6  307.7   1114.9  
5      285.7   62.3    223.2     5.0x 
10     143.4   31.2    112.1     9.9x 
20      71.4   15.6     55.8    19.9x 
40      37.1    7.9     29.1    38.4x 
68      23.1    4.8     18.3    61.6x 
136     17.1    3.4     13.6    83.2x 
272     19.3    3.2     16.0    73.7x 

friendster 
----------------------------------------
#p     total    ppt      tct   speedup  
1     1618.5  421.3   1196.7   
5      316.8   84.8    231.5     5.1x  
10     159.0   42.5    116.1    10.2x 
20      79.5   21.3     57.8    20.4x
40      40.6   10.7     29.4    39.9x 
68      25.5    6.5     18.4    63.5x 
136     16.9    4.5     11.7    95.8x 
272     13.5    3.6      8.9   119.9x 
```

## Citing 
The parallel algorithm implemented is based on the one described in

["Exploring Optimizations on Shared-memory Platforms for Parallel Triangle Counting
Algorithms". Ancy Sarah Tom, Narayanan Sundaram, Nesreen K. Ahmed, Shaden Smith, 
Stijn Eyerman, Midhunchandra Kodiyath, Ibrahim Hur, Fabrizio Petrini, and George
Karypis. IEEE High Performance Extreme Computing Conference (HPEC),
2017](http://glaros.dtc.umn.edu/gkhome/node/1214), which was one of the finalists for
the [GraphChallenge 2017 competition](http://graphchallenge.mit.edu/).

