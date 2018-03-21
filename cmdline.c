/*!
\file  
\brief Parsing of command-line arguments
 
\date   Started 5/10/17
\author George
\version\verbatim $Id: cmdline.c 21866 2018-03-04 22:27:13Z karypis $ \endverbatim
*/


#include "tc.h"

/*-------------------------------------------------------------------
 * Command-line options 
 *-------------------------------------------------------------------*/
static struct gk_option long_options[] = {
  {"iftype",            1,      0,      CMD_IFTYPE},
  {"nthreads",          1,      0,      CMD_NTHREADS},
  {"help",              0,      0,      CMD_HELP},
  {0,                   0,      0,      0}
};


static gk_StringMap_t iftype_options[] = {
  {"tsv",         IFTYPE_TSV},
  {"metis",       IFTYPE_METIS},
  {NULL,                 0}
};


/*-------------------------------------------------------------------
 * Mini help
 *-------------------------------------------------------------------*/
static char helpstr[][100] =
{
" ",
"Usage: gktc [options] infile",
" ",        
" Options",
"  -iftype=text",
"     Specifies the format of the input file. ",
"     Possible values are:",
"        metis   Metis format [default]",
"        tsv     tsv format (i, j, v)",
" ",
"  -nthreads=int",
"     Specifies the number of threads to use.",
"     The default value is set to the value returned by omp_get_max_threads().",
" ",
"  -help",
"     Prints this message.",
""
};

 

/*************************************************************************/
/*! This is the entry point of the command-line argument parser */
/*************************************************************************/
params_t *getcmdline_params(int argc, char *argv[])
{
  gk_idx_t i, j, k;
  int type=0;
  int c, option_index;
  params_t *params;

  params = (params_t *)gk_malloc(sizeof(params_t), "cmdline_parse: params");
  memset(params, 0, sizeof(params_t)); 

  /* print the command line */
  for (i=0; i<argc; i++)
    printf("%s ", argv[i]);
  printf("\n");

  /* initialize the params data structure */
  params->infile   = NULL;
  params->iftype   = IFTYPE_METIS;
  params->nthreads = 1;

#if defined(_OPENMP)
  params->nthreads = omp_get_max_threads();
#endif

  /* Parse the command line arguments  */
  while ((c = gk_getopt_long_only(argc, argv, "", long_options, &option_index)) != -1) {
    switch (c) {
      case CMD_IFTYPE:
        if (gk_optarg) {
          if ((params->iftype = gk_GetStringID(iftype_options, gk_optarg)) == -1) 
            errexit("Invalid iftype of %s.\n", gk_optarg);
        }
        break; 

      case CMD_NTHREADS:
        if (gk_optarg) {
          if ((params->nthreads = atoi(gk_optarg)) < 1)
            errexit("The -nthreads must be greater than 0.\n");
        }
        break;

      case CMD_HELP:
        for (i=0; strlen(helpstr[i]) > 0; i++)
          printf("%s\n", helpstr[i]);
        exit(EXIT_SUCCESS);
        break;

      default:
        printf("Illegal command-line option(s)\nUse %s -help for a summary of the options.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  /* Get the operation to be performed */
  if (argc-gk_optind != 1) {
    printf("Missing required parameters.\n  Use %s -help for a summary of the options.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  params->infile = gk_strdup(argv[gk_optind++]);

  return params;
}
