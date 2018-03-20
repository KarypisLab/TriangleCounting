/*!
\file
\brief This file contains function prototypes
\date Started 5/12/2017
\author George
\version\verbatim $Id: proto.h 21908 2018-03-20 02:14:37Z karypis $ \endverbatim
*/

#ifndef _PROTO_H_
#define _PROTO_H_

/* io.c */
vault_t *loadData(params_t *params);

/* cmdline.c */
params_t *getcmdline_params(int argc, char *argv[]);

/* ptc.c */
gk_graph_t *ptc_Preprocess(params_t *params, vault_t *vault);
int64_t ptc_MapJIK(params_t *params, vault_t *vault);

#endif
