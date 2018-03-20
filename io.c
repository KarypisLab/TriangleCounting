/*!
\file
\brief This file contains routines for reading in the data set
\date Started 5/10/2017
\author George
\version\verbatim $Id: io.c 21013 2017-05-19 03:08:00Z karypis $ \endverbatim
*/

#include "tc.h"


/**************************************************************************/
/*! Reads the input data */
/**************************************************************************/
vault_t *loadData(params_t *params)
{
  vault_t *vault;

  vault = (vault_t *)gk_malloc(sizeof(vault_t), "loadData: vault");
  memset(vault, 0, sizeof(vault_t));

  /* read the graph */
  printf("Reading graph %s...\n", params->infile);
  GKASSERT(gk_fexists(params->infile)); 

  switch (params->iftype) {
    case IFTYPE_TSV:
      vault->graph = gk_graph_Read(params->infile, GK_GRAPH_FMT_IJV, 1, 1, 0, 0, 0);
      gk_free((void **)&(vault->graph->iadjwgt), LTERM);
      break;

    case IFTYPE_METIS:
      vault->graph = gk_graph_Read(params->infile, GK_GRAPH_FMT_METIS, -1, -1, 0, 0, 0);
      break;

    default:
      errexit("Unknown iftype of %d\n", params->iftype);
  }

  return vault;
}

