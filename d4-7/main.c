#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "d4.h"
#include "cmdd4.h"
#include "cmdargs.h"
#include "tracein.h"
#include "global.h"

extern G* gg[];

extern int do_cache_init(int core_id);
extern int do_cache_ref(int core_id, d4memref r);
extern void summarize_caches (G *g, d4cache *ci, d4cache *cd);
extern d4memref next_trace_item(G *g);

/*
 * Everything starts here
 */
int
main (int argc, char **argv)
{
	d4memref r;
	//printf("%s %d ---------------------------\n", __FUNCTION__, __LINE__);
	int core = do_cache_init(0);
	//printf("%s %d ---------------------------\n", __FUNCTION__, __LINE__);
	int core1 = do_cache_init(1);
	//printf("%s %d ---------------------------\n", __FUNCTION__, __LINE__);
	if (core < 0) {
		printf("ERROR: fail to init cache\n");
		return -1;
	}

	G *g = gg[core];
	G *g1 = gg[core1];

	printf ("---Dinero IV cache simulator, version %s\n", D4VERSION);
	printf ("---Written by Jan Edler and Mark D. Hill\n");
	printf ("---Copyright (C) 1997 NEC Research Institute, Inc. and Mark D. Hill.\n");
	printf ("---All rights reserved.\n");
	printf ("---Copyright (C) 1985, 1989 Mark D. Hill.  All rights reserved.\n");
	printf ("---See -copyright option for details\n");

	summarize_caches (g, g->ci, g->cd);

	int miss_cnt = 0;

	printf ("\n---Simulation begins.\n");
	// g->tmaxcount = 0;
	// g->tintcount = g->stat_interval;
	// g->flcount = g->flushcount;
	while (1) {
		r = next_trace_item(g1);
		//printf("%s %d\n", __FUNCTION__, __LINE__);
		miss_cnt = do_cache_ref(core1, r);
		if (miss_cnt == -1) goto done;

		r = next_trace_item(g);
		//printf("%s %d\n", __FUNCTION__, __LINE__);
		miss_cnt = do_cache_ref(core, r);
		if (miss_cnt == -1) goto done;
	}

done:
	/* copy everything back at the end -- is this really a good idea? XXX */
	r.accesstype = D4XCOPYB;
	r.address = 0;
	r.size = 0;
	miss_cnt = d4ref (g, g->cd, r); printf("miss %d\n", miss_cnt);
	miss_cnt = d4ref (g1, g1->cd, r); printf("g1: miss %d\n", miss_cnt);
	printf ("---Simulation complete.\n");
	dostats(g);
	printf ("---Simulation complete.\n");
	dostats(g1);
	printf ("---Execution complete.\n");
	return 0;
}
