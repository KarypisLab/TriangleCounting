/*!
\file
\brief Data structures used in the program
\date Started 5/10/2017
\author George
\version\verbatim $Id: struct.h 21866 2018-03-04 22:27:13Z karypis $ \endverbatim
*/

#ifndef _STRUCT_TC_H_
#define _STRUCT_TC_H_


/*************************************************************************
* the data vault 
**************************************************************************/
typedef struct {
  gk_graph_t *graph;      /* the graph */

  uint64_t nprobes;       /* the number of intersections that are checked */

  /* timers */
  double timer_global;
  double timer_pp;
  double timer_tc;
} vault_t; 


/*************************************************************************
* run parameters 
**************************************************************************/
typedef struct {
  int iftype;           /* The format of the input file */
  int nthreads;         /* The number of threads for the OpenMP variants */

  char *infile;         /* The file storing the input data */

  vault_t *vault;
} params_t;



#endif 
