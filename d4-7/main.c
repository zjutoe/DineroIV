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

/*
 * Everything starts here
 */
int
main (int argc, char **argv)
{
	d4memref r;
	G *g = (G*)malloc(sizeof(G));	
	if (g == NULL)
		printf("g malloc failed\n");
	double tmaxcount = 0, tintcount;
	double flcount;
	g->progname = "dineroIV";
	g->cust_argc = 1;
	g->informat = DEFVAL_informat;

	// init_args(g, g->args);
	// memcpy(g->args, args, nargs * sizeof(args[0]));

	if (argc > 0) {
		char *cp;
		g->progname = argv[0];
		while ((cp = strrchr (g->progname, '/')) != NULL) {
			if (cp[1] == 0)
				cp[0] = 0;	/* trim trailing '/' */
			else
				g->progname = cp+1;
		}
	}
	//doargs (g, argc, argv);
	doargs_simple(g, argc, argv);
	verify_options(g);
	initialize_caches (g, &g->ci, &g->cd);

#if !D4CUSTOM
	if (g->customname != NULL) {
		customize_caches(g);
		/* never returns */
	}
#endif
	if (g->cd == NULL)
		g->cd = g->ci;	/* for unified L1 cache */

	printf ("---Dinero IV cache simulator, version %s\n", D4VERSION);
	printf ("---Written by Jan Edler and Mark D. Hill\n");
	printf ("---Copyright (C) 1997 NEC Research Institute, Inc. and Mark D. Hill.\n");
	printf ("---All rights reserved.\n");
	printf ("---Copyright (C) 1985, 1989 Mark D. Hill.  All rights reserved.\n");
	printf ("---See -copyright option for details\n");

	summarize_caches (g, g->ci, g->cd);

	int miss_cnt = 0;

	printf ("\n---Simulation begins.\n");
	tintcount = g->stat_interval;
	flcount = g->flushcount;
	while (1) {
		miss_cnt = do_cache_ref(g, r, g->ci, g->cd, &tmaxcount, &flcount, tintcount), &tmaxcount;
		if (miss_cnt == -1) goto done;
	}
done:
	/* copy everything back at the end -- is this really a good idea? XXX */
	r.accesstype = D4XCOPYB;
	r.address = 0;
	r.size = 0;
	miss_cnt = d4ref (g->cd, r); printf("miss %d\n", miss_cnt);
	printf ("---Simulation complete.\n");
	dostats(g);
	printf ("---Execution complete.\n");
	return 0;
}
