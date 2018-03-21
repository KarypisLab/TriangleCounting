# TriangleCounting
OpenMP-based parallel program for counting the number of triangles in a sparse graph.


## Build requirements
 - CMake 2.8, found at http://www.cmake.org/, as well as GNU make. 
 - Download, build, and install [GKlib](https://github.com/KarypisLab/GKlib).

Assuming that the above are available, two commands should suffice to 
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
openmp=not-set    - To build a serial version
```

## Building and installing
To build and install, run the following
```
make
make install
```

## Other make commands
    make uninstall 
         Removes all files installed by 'make install'.
   
    make clean 
         Removes all object files but retains the configuration options.
   
    make distclean 
         Performs clean and completely removes the build directory.


## Runing the program 
By default, the binary, called _gktc_, will be installed in ~/local/bin.
For usage information just type
```
gktc -help

Usage: gktc [options] infile

 Options
  -iftype=text
     Specifies the format of the input file.
     Possible values are:
        metis   Metis format [default]
        tsv     tsv format (i, j, v)

  -nthreads=int
     Specifies the number of threads to use.
     The default value is set to the value returned by omp_get_max_threads().

  -help
     Prints this message.
```

The program supports two formats for its input files: 
- The one used by the [Metis](http://www.cs.umn.edu/~metis) graph 
  partitioning program.
- The tsv format used by the graphs in the 
  [GraphChallenge 2017](http://graphchallenge.mit.edu/) competition.
Note that the graph has to be undirected and it needs to include both pairs of
edges (i.e., (u,v) and (v,u)).

Here is the output of a sample run:
```
gktc -nthreads=4 test/p2p-Gnutella31.metis
Reading graph test/p2p-Gnutella31.metis...

-----------------
  infile: test/p2p-Gnutella31.metis
  #nvtxs: 62586
 #nedges: 295784
nthreads: 4

& compatible maxhmsize: 255, startv: 23

Results...
  #triangles:         2024; #probes:       209251; rate:     248.56 MP/sec

Timings...
     preprocessing:     0.002s
 triangle counting:     0.001s
    total (/x i/o):     0.003s
-----------------
```


## Performance 
The following shows a sample of gktc's performance on Intel's Knights Landing
processor:

```
     #p: # of threads (this KNL has 68 cores)
  total: total time excluding I/O
    ppt: pre-processing time
    tct: triangle counting time
speedup: relative to p=1

rmat scale25 
----------------------------------------
#p     total     ppt       tct   speedup 
1      767.4s  141.9s    625.5s   
5      154.4s   29.1s    125.2s     5.0x 
10      77.2s   14.5s     62.6s     9.9x
20      38.6s    7.3s     31.2s    19.9x
40      19.6s    3.7s     16.1s    39.2x
68      12.2s    2.2s      9.9s    62.9x
136      9.3s    1.7s      7.6s    82.5x
272     10.1s    1.5s      8.6s    76.0x

twitter 
----------------------------------------
#p     total     ppt       tct   speedup    
1     1422.6s  307.7s   1114.9s  
5      285.7s   62.3s    223.2s     5.0x 
10     143.4s   31.2s    112.1s     9.9x 
20      71.4s   15.6s     55.8s    19.9x 
40      37.1s    7.9s     29.1s    38.4x 
68      23.1s    4.8s     18.3s    61.6x 
136     17.1s    3.4s     13.6s    83.2x 
272     19.3s    3.2s     16.0s    73.7x 

friendster 
----------------------------------------
#p     total     ppt       tct   speedup  
1     1618.5s  421.3s   1196.7s   
5      316.8s   84.8s    231.5s     5.1x  
10     159.0s   42.5s    116.1s    10.2x 
20      79.5s   21.3s     57.8s    20.4x
40      40.6s   10.7s     29.4s    39.9x 
68      25.5s    6.5s     18.4s    63.5x 
136     16.9s    4.5s     11.7s    95.8x 
272     13.5s    3.6s      8.9s   119.9x 
```

## Citing 
The parallel algorithm implemented is based on the one described in

[__"Exploring Optimizations on Shared-memory Platforms for Parallel Triangle Counting
Algorithms."__ Ancy Sarah Tom, Narayanan Sundaram, Nesreen K. Ahmed, Shaden Smith, 
Stijn Eyerman, Midhunchandra Kodiyath, Ibrahim Hur, Fabrizio Petrini, and George
Karypis. IEEE High Performance Extreme Computing Conference (HPEC),
2017](http://glaros.dtc.umn.edu/gkhome/node/1214)

This was one of the finalists for the [GraphChallenge
2017](http://graphchallenge.mit.edu/) competition.

