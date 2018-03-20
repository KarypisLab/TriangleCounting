/*!
\file
\brief The entry point of the triangle counting code
\date Started 5/10/2017
\author George
\version\verbatim $Id: cmdline.c 20946 2017-05-10 23:12:48Z karypis $ \endverbatim
*/

#include "tc.h"

/*************************************************************************
* The entry point 
**************************************************************************/
int main(int argc, char *argv[])
{
  int64_t ntriangles=0;
  params_t *params;
  vault_t *vault;

  params = getcmdline_params(argc, argv);

  vault = params->vault = loadData(params);
  vault->nprobes = 0;

#if defined(_OPENMP)
  omp_set_num_threads(params->nthreads);
  omp_set_nested(0);
  omp_set_dynamic(0);
#endif

  printf("\n-----------------\n");
  printf("  infile: %s\n", params->infile);
  printf("  #nvtxs: %d\n", vault->graph->nvtxs);
  printf(" #nedges: %zd\n", vault->graph->xadj[vault->graph->nvtxs]);
#if defined(_OPENMP)
  printf("nthreads: %d\n", omp_get_max_threads());
#else
  params->nthreads = 1;
  printf("nthreads: 1 (was not compiled with openmp support)\n");
#endif
  printf("\n");


  gk_startwctimer(vault->timer_global);
  ntriangles = ptc_MapJIK(params, vault);
  gk_stopwctimer(vault->timer_global);

  printf("\nResults...\n");
  printf("  #triangles: %12"PRId64"; #probes: %12"PRIu64"; rate: %10.2lf MP/sec\n", 
      ntriangles, vault->nprobes, 
      ((double)vault->nprobes)/((double)1e6*gk_getwctimer(vault->timer_tc))); 

  printf("\nTimings...\n");
  printf("     preprocessing: %9.3lfs\n", gk_getwctimer(vault->timer_pp));
  printf(" triangle counting: %9.3lfs\n", gk_getwctimer(vault->timer_tc));
  printf("    total (/x i/o): %9.3lfs\n", gk_getwctimer(vault->timer_global));
  printf("-----------------\n");

}

