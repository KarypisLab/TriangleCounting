/*!
\file
\brief The OpenMP triangle counting routine
\date Started 1/14/2018
\author George
\version\verbatim $Id: cmdline.c 20946 2017-05-10 23:12:48Z karypis $ \endverbatim
*/

#include "tc.h"

#define SBSIZE  64
#define DBSIZE  8


/*************************************************************************/
/*! Reorders the vertices in the graph in inc degree order and returns
    the re-ordered graph in which the adjancency lists are sorted in 
    increasing order. In addition, a diagonal entry is added to each
    row to make the code that follows tighter.
*/
/*************************************************************************/
gk_graph_t *ptc_Preprocess(params_t *params, vault_t *vault)
{
  int32_t vi, nvtxs, nthreads, maxdegree=0, csrange=0;
  ssize_t *xadj, *nxadj, *psums;
  int32_t *adjncy, *nadjncy, *perm=NULL, *iperm=NULL, *chunkptr=NULL;
  int32_t *gcounts;
  gk_graph_t *graph;

  nthreads = params->nthreads;

  nvtxs  = vault->graph->nvtxs;
  xadj   = vault->graph->xadj;
  adjncy = vault->graph->adjncy;

  graph = gk_graph_Create();
  graph->nvtxs  = nvtxs;
  graph->xadj   = nxadj = gk_zmalloc(nvtxs+1, "nxadj");
  graph->adjncy = nadjncy = gk_i32malloc(nvtxs+xadj[nvtxs], "nadjncy");

  perm  = gk_i32malloc(nvtxs, "perm");   /* perm[old-vtx-num]  => new-vtx-num */
  iperm = gk_i32malloc(nvtxs, "iperm");  /* iperm[new-vtx-num] => old-vtx-num */

  /* Determine maxdegree/csrange */
  #pragma omp parallel for schedule(static,4096) default(none) \
     shared(nvtxs, xadj) \
     reduction(max: maxdegree)
  for (vi=0; vi<nvtxs; vi++) 
    maxdegree = gk_max(maxdegree, (int32_t)(xadj[vi+1]-xadj[vi]));

  csrange = maxdegree+1; 
  csrange = 16*((csrange+15)/16); /* get the per thread arrays to be alligned 
                                     at the start of the cache line */

  gcounts = gk_i32malloc(nthreads*csrange, "gcounts");
  psums   = gk_zmalloc(nthreads, "psums");

  #pragma omp parallel default(none) \
     shared(vault, nvtxs, nthreads, maxdegree, csrange, xadj, adjncy, nxadj, nadjncy, \
            perm, iperm, gcounts, psums, chunkptr, stdout) 
  {
    int32_t vi, vistart, viend, vj, nedges, nchunks;
    int32_t ti, di, ci, dstart, dend;
    int32_t *counts, *buffer;
    ssize_t ej, ejend, psum, chunksize;
#if defined(_OPENMP)
    int mytid = omp_get_thread_num();
#else
    int mytid = 0;
#endif

    vistart = mytid*((nvtxs+nthreads-1)/nthreads);
    viend   = gk_min(nvtxs, (mytid+1)*((nvtxs+nthreads-1)/nthreads));

    dstart = mytid*((csrange+nthreads-1)/nthreads);
    dend   = gk_min(csrange, (mytid+1)*((csrange+nthreads-1)/nthreads));

    /* Compute the local counts */
    counts = gcounts + mytid*csrange;
    gk_i32set(csrange, 0, counts);

    for (vi=vistart; vi<viend; vi++) 
      counts[xadj[vi+1]-xadj[vi]]++;
    #pragma omp barrier

    /* Compute the partial sum of the range assigned to each thread */
    for (psum=0, ti=0; ti<nthreads; ti++) {
      counts = gcounts + ti*csrange;
      for (di=dstart; di<dend; di++) 
        psum += counts[di];
    }
    psums[mytid] = psum;
    #pragma omp barrier

    #pragma omp single
    for (ti=1; ti<nthreads; ti++)
      psums[ti] += psums[ti-1];
    #pragma omp barrier

    /* Compute the actual prefix sums of the range assigned to each thread.
       This is done from right to left to get it into the desired exclusive
       prefix sum stage. */
    psum = psums[mytid];
    for (di=dend-1; di>=dstart; di--) { 
      counts = gcounts + (nthreads-1)*csrange;
      for (ti=nthreads-1; ti>=0; ti--) {
        psum -= counts[di];
        counts[di] = psum;
        counts -= csrange;
      }
    }
    #pragma omp barrier

    /* Create the perm/iperm arrays and the nxadj array of the re-ordered graph */
    counts = gcounts + mytid*csrange;

    /* TODO: This can be optimized by pre-sorting the per-thread vertices according 
             to their degree and processing them in increasing degree order */
    for (vi=vistart; vi<viend; vi++) {
      perm[vi] = counts[xadj[vi+1]-xadj[vi]]++;
      nxadj[perm[vi]] = xadj[vi+1]-xadj[vi]+1; /* the +1 is for the diagonal */
      iperm[perm[vi]] = vi;
    }
    #pragma omp barrier

    #pragma omp barrier
    /* compute the local sums and their prefix sums */
    for (psum=0, vi=vistart; vi<viend; vi++)
      psum += nxadj[vi];
    psums[mytid] = psum;
    #pragma omp barrier

    #pragma omp single
    for (ti=1; ti<nthreads; ti++)
      psums[ti] += psums[ti-1];
    #pragma omp barrier

    /* Compute the actual prefix sums of the nxadj[] array assigned to each thread.
       This is done from right to left to get it into the desired exclusive
       prefix sum stage. */
    psum = psums[mytid];
    if (mytid == nthreads-1)
      nxadj[nvtxs] = psum;
    for (vi=viend-1; vi>=vistart; vi--) { 
      psum -= nxadj[vi];
      nxadj[vi] = psum;
    }
    #pragma omp barrier
      
    /* Compute the chunk-based partitioning of the work for the reordered/sorted graph */
    chunksize = 1+psums[nthreads-1]/(100*nthreads);
    for (nchunks=0, psum=0, vi=vistart; vi<viend; vi++) {
      if ((psum += nxadj[vi+1]-nxadj[vi]) >= chunksize) {
        nchunks++;
        psum = 0;
      }
    }
    psums[mytid] = nchunks+1;

    #pragma omp barrier
    #pragma omp single
    for (ti=1; ti<nthreads; ti++)
      psums[ti] += psums[ti-1];
    #pragma omp barrier

    #pragma omp single
    chunkptr = gk_i32malloc(psums[nthreads-1]+1, "chunkptr");
    #pragma omp barrier

    nchunks = psums[mytid];
    chunkptr[nchunks] = viend;
    for (psum=0, vi=viend-1; vi>=vistart; vi--) {
      if ((psum += nxadj[vi+1]-nxadj[vi]) >= chunksize) {
        chunkptr[--nchunks] = vi;
        psum = 0;
      }
    }
    if (mytid == 0)
      chunkptr[0] = 0;
    #pragma omp barrier

    nchunks = psums[nthreads-1]; /* this is the total # of chunks */
    /*
    #pragma omp single
    {
      for (vi=0; vi<nchunks; vi++) {
        printf("%4d: %6d - %6d [%5d: %zd]\n", vi, chunkptr[vi], chunkptr[vi+1], 
            chunkptr[vi+1]-chunkptr[vi], nxadj[chunkptr[vi+1]]-nxadj[chunkptr[vi]]);
      }
    }
    #pragma omp barrier
    */

    /* create the reordered/sorted graph by processing the chunks in parallel */
    #pragma omp for schedule(dynamic, 1) nowait
    for (ci=nchunks-1; ci>=0; ci--) {
      for (vi=chunkptr[ci]; vi<chunkptr[ci+1]; vi++) {
        vj = iperm[vi];
        buffer = nadjncy+nxadj[vi];
        for (nedges=0, ej=xadj[vj], ejend=xadj[vj+1]; ej<ejend; ej++, nedges++) 
          buffer[nedges] = perm[adjncy[ej]];
        buffer[nedges++] = vi; /* put the diagonal */

        if (nedges > 1)
          gk_i32sorti(nedges, buffer);  /* sort adjncy list */
      }
    }

  }

  gk_free((void **)&perm, &iperm, &gcounts, &psums, &chunkptr, LTERM);

  return graph;
}


/*************************************************************************/
/*! The hash-map-based triangle-counting routine that uses the JIK
    triangle enumeration scheme.

    This version implements the following:
     - It does not store location information in L
     - Adds diagonal entries in U to make loops tighter
     - Reverts the order within U's adjancency lists to allow ++ traversal
*/
/*************************************************************************/
int64_t ptc_MapJIK(params_t *params, vault_t *vault)
{
  int32_t vi, vj, nvtxs, startv;
  ssize_t ei, ej;
  int64_t ntriangles=0;
  ssize_t *xadj, *uxadj;
  int32_t *adjncy;
  int32_t l2, maxhmsize=0;
  gk_graph_t *graph;
  uint64_t nprobes=0;

  gk_startwctimer(vault->timer_pp);
  graph = ptc_Preprocess(params, vault);
  gk_stopwctimer(vault->timer_pp);

  nvtxs  = graph->nvtxs;
  xadj   = graph->xadj;
  adjncy = graph->adjncy;

  uxadj = gk_zmalloc(nvtxs, "uxadj"); /* the locations of the upper triangular part */

  gk_startwctimer(vault->timer_tc);

  /* populate uxadj[] and determine the size of the hash-map */
  startv = nvtxs;
  #pragma omp parallel for schedule(dynamic,1024) \
     default(none) \
     shared(nvtxs, xadj, adjncy, uxadj) \
     private(vj, ei, ej) \
     reduction(max: maxhmsize) \
     reduction(min: startv)
  for (vi=nvtxs-1; vi>=0; vi--) {
    for (ei=xadj[vi+1]-1; adjncy[ei]>vi; ei--); 
    uxadj[vi] = ei;
    maxhmsize = gk_max(maxhmsize, (int32_t)(xadj[vi+1]-uxadj[vi]));
    startv = (uxadj[vi] != xadj[vi] ? vi : startv);

    /* flip the order of Adj(vi)'s upper triangular adjacency list */
    for (ej=xadj[vi+1]-1; ei<ej; ei++, ej--) {
      vj = adjncy[ei];
      adjncy[ei] = adjncy[ej];
      adjncy[ej] = vj;
    }
  }

  /* convert the hash-map is converted into a format that is compatible with a 
     bitwise AND operation */
  for (l2=1; maxhmsize>(1<<l2); l2++);
  maxhmsize = (1<<(l2+4))-1;

  printf("& compatible maxhmsize: %"PRId32", startv: %d\n", maxhmsize, startv);

  #pragma omp parallel default(none) \
    shared(params, vault, nvtxs, xadj, adjncy, uxadj, maxhmsize, startv, stdout) \
    reduction(+: ntriangles, nprobes)
  {
    int32_t vi, vj, vk, vl, nlocal;
    ssize_t ei, eiend, eistart, ej, ejend, ejstart;
    int32_t l, nc;
    int32_t l2=1, hmsize=(1<<(l2+4))-1, *hmap;
#if defined(_OPENMP)
    int mytid = omp_get_thread_num();
#else
    int mytid = 0;
#endif

    hmap = gk_i32smalloc(maxhmsize+1, 0, "hmap");

    /* Phase 1: Count triangles for vj < nvtxs-maxhmsize */
    #pragma omp for schedule(dynamic,SBSIZE) nowait
    for (vj=startv; vj<nvtxs-maxhmsize; vj++) {
      if (xadj[vj+1]-uxadj[vj] == 1 || xadj[vj] == uxadj[vj])
        continue;
  
      /* adjust hmsize if needed */
      if (xadj[vj+1]-uxadj[vj] > (1<<l2)) {
        for (++l2; (xadj[vj+1]-uxadj[vj])>(1<<l2); l2++);
        hmsize = (1<<(l2+4))-1;
      }

      /* hash Adj(vj) */
      for (nc=0, ej=uxadj[vj], ejend=xadj[vj+1]-1; ej<ejend; ej++) {
        vk = adjncy[ej];
        for (l=(vk&hmsize); hmap[l]!=0; l=((l+1)&hmsize), nc++);
        hmap[l] = vk;
      }
  
      nlocal = 0;

      /* find intersections */
      if (nc > 0) { /* we had collisions */
        for (ej=xadj[vj], ejend=uxadj[vj]; ej<ejend; ej++) {
          vi = adjncy[ej];
          for (ei=uxadj[vi]; adjncy[ei]>vj; ei++) {
            vk = adjncy[ei];
            for (l=vk&hmsize; hmap[l]!=0 && hmap[l]!=vk; l=((l+1)&hmsize));
            if (hmap[l] == vk) 
              nlocal++;
          }
          nprobes += ei-uxadj[vi];
        }
  
        /* reset hash */
        for (ej=uxadj[vj], ejend=xadj[vj+1]-1; ej<ejend; ej++) {
          vk = adjncy[ej];
          for (l=(vk&hmsize); hmap[l]!=vk; l=((l+1)&hmsize));
          hmap[l] = 0;
        }
      }
      else { /* there were no collisons */
        for (ej=xadj[vj], ejend=uxadj[vj]; ej<ejend; ej++) {
          vi = adjncy[ej];
#ifdef TC_VECOPT 
          for (eiend=uxadj[vi]; adjncy[eiend]>vj; eiend++);
          for (ei=uxadj[vi]; ei<eiend; ei++) 
#else
          for (ei=uxadj[vi]; adjncy[ei]>vj; ei++) 
#endif
          {
            vk = adjncy[ei];
            nlocal += (hmap[vk&hmsize] == vk);
          }
          nprobes += ei-uxadj[vi];
        }
  
        /* reset hash */
        for (ej=uxadj[vj], ejend=xadj[vj+1]-1; ej<ejend; ej++) 
          hmap[adjncy[ej]&hmsize] = 0;
      }
      
      if (nlocal > 0)
        ntriangles += nlocal;
    }


    /* Phase 2: Count triangles for the last hmsize vertices, which can be done
                faster by using hmap as a direct map array. */
    hmap -= (nvtxs - maxhmsize);
    #pragma omp for schedule(dynamic,DBSIZE) nowait
    for (vj=nvtxs-1; vj>=nvtxs-maxhmsize; vj--) {
      if (xadj[vj+1]-uxadj[vj] == 1 || xadj[vj] == uxadj[vj])
        continue;
  
      nlocal = 0;

      if (xadj[vj+1]-uxadj[vj] == nvtxs-vj) { /* complete row */
        /* find intersections */
        for (ej=xadj[vj], ejend=uxadj[vj]; ej<ejend; ej++) {
          vi = adjncy[ej];
          for (ei=uxadj[vi]; adjncy[ei]>vj; ei++);
          nlocal  += ei-uxadj[vi];
          nprobes += ei-uxadj[vi];
        }
      }
      else {
        /* hash Adj(vj) */
        for (ej=uxadj[vj], ejend=xadj[vj+1]-1; ej<ejend; ej++) 
          hmap[adjncy[ej]] = 1;
  
        /* find intersections */
        for (ej=xadj[vj], ejend=uxadj[vj]; ej<ejend; ej++) {
          vi = adjncy[ej];
#ifdef TC_VECOPT 
          for (eiend=uxadj[vi]; adjncy[eiend]>vj; eiend++);
          for (ei=uxadj[vi]; ei<eiend; ei++) 
#else
          for (ei=uxadj[vi]; adjncy[ei]>vj; ei++) 
#endif
            nlocal += hmap[adjncy[ei]];
          nprobes += ei-uxadj[vi];
        }
  
        /* reset hash */
        for (ej=uxadj[vj], ejend=xadj[vj+1]-1; ej<ejend; ej++) 
          hmap[adjncy[ej]] = 0;
      }
  
      if (nlocal > 0)
        ntriangles += nlocal;
    }
    hmap += (nvtxs - maxhmsize);

    gk_free((void **)&hmap, LTERM);
  }

  gk_stopwctimer(vault->timer_tc);

  gk_graph_Free(&graph);
  gk_free((void **)&uxadj, LTERM);

  vault->nprobes = nprobes;

  return ntriangles;
}

