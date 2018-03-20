/*!
\file
\brief This file contains various constant definitions
\date Started 5/10/17
\author George
\version\verbatim $Id: defs.h 21907 2018-03-20 02:12:02Z karypis $ \endverbatim
*/

#ifndef __DEF_H__
#define __DEF_H__

#define MAXLINE         1024*128
#define MAX_STRLEN      1024*128

#ifdef __AVX512CD__
  #define TC_VECOPT 1
#endif

/* command-line options */
#define CMD_IFTYPE              1
#define CMD_NTHREADS            2
#define CMD_HELP                200

/* iftype */
#define IFTYPE_TSV              1
#define IFTYPE_METIS            2


/* The text labels for the different iftypes */
static char iftypenames[][10] = 
                {"", "tsv", "metis", ""}; 

#endif

