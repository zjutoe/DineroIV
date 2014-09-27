/*
 * Declarations for argument handling for Dinero IV's command interface.
 * Written by Jan Edler
 *
 * Copyright (C) 1997 NEC Research Institute, Inc. and Mark D. Hill.
 * All rights reserved.
 * Copyright (C) 1985, 1989 Mark D. Hill.  All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its associated documentation for non-commercial purposes is hereby
 * granted (for commercial purposes see below), provided that the above
 * copyright notice appears in all copies, derivative works or modified
 * versions of the software and any portions thereof, and that both the
 * copyright notice and this permission notice appear in the documentation.
 * NEC Research Institute Inc. and Mark D. Hill shall be given a copy of
 * any such derivative work or modified version of the software and NEC
 * Research Institute Inc.  and any of its affiliated companies (collectively
 * referred to as NECI) and Mark D. Hill shall be granted permission to use,
 * copy, modify, and distribute the software for internal use and research.
 * The name of NEC Research Institute Inc. and its affiliated companies
 * shall not be used in advertising or publicity related to the distribution
 * of the software, without the prior written consent of NECI.  All copies,
 * derivative works, or modified versions of the software shall be exported
 * or reexported in accordance with applicable laws and regulations relating
 * to export control.  This software is experimental.  NECI and Mark D. Hill
 * make no representations regarding the suitability of this software for
 * any purpose and neither NECI nor Mark D. Hill will support the software.
 * 
 * Use of this software for commercial purposes is also possible, but only
 * if, in addition to the above requirements for non-commercial use, written
 * permission for such use is obtained by the commercial user from NECI or
 * Mark D. Hill prior to the fabrication and distribution of the software.
 * 
 * THE SOFTWARE IS PROVIDED AS IS.  NECI AND MARK D. HILL DO NOT MAKE
 * ANY WARRANTEES EITHER EXPRESS OR IMPLIED WITH REGARD TO THE SOFTWARE.
 * NECI AND MARK D. HILL ALSO DISCLAIM ANY WARRANTY THAT THE SOFTWARE IS
 * FREE OF INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OF OTHERS.
 * NO OTHER LICENSE EXPRESS OR IMPLIED IS HEREBY GRANTED.  NECI AND MARK
 * D. HILL SHALL NOT BE LIABLE FOR ANY DAMAGES, INCLUDING GENERAL, SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, ARISING OUT OF THE USE OR INABILITY
 * TO USE THE SOFTWARE.
 *
 * $Header: /home/edler/dinero/d4/RCS/cmdargs.h,v 1.5 1997/12/08 19:35:24 edler Exp $
 */

#include "global.h"

// /* Some globals, defined in cmdargs.c */
// extern struct arglist args[];	/* defined in cmdargs.c */
// extern int nargs;		/* num entries in args[] */
// extern int maxlevel;		/* largest cache level specified */
// extern int optstringmax;	/* longest option string */

// extern char *customname;	/* for -custom, name of executable */
// extern double skipcount;	/* for -skipcount */
// extern double flushcount;	/* for -flushcount */
// extern double maxcount;		/* for -maxcount */
// extern double stat_interval;	/* for -stat-interval */
// extern long on_trigger;		/* for -on-trigger */
// extern long off_trigger;	/* for -off-trigger */
// extern int stat_idcombine;	/* for -stat-idcombine */

extern void init_args(G *g, struct arglist args[]);
/*
 * Helper for match and value functions recognizing -ln-idu prefix
 */
extern char *level_idu (G *g, const char *opt, int *level, int *idu);

/*
 * Helpers for options with scaled arguments
 */
extern unsigned int argscale_uint (const char *arg, unsigned int *var);
extern double argscale_uintd (const char *arg, double *var);

/*
 * Match functions -- recognize the arg on the command line
 * Return the number of argv elements to be consumed.
 */
extern int match_0arg (G *g, const char *, const struct arglist *);
extern int pmatch_0arg (G *g, const char *, const struct arglist *);
extern int match_1arg (G *g, const char *, const struct arglist *);
extern int pmatch_1arg (G *g, const char *, const struct arglist *); /* with -ln-idu prefix */
#if D4CUSTOM
extern int match_bogus (const char *, const struct arglist *);
#endif

/*
 * value functions -- actually consume the option and argument(s).
 */
extern void val_help (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_helpcr (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_helpw (G *g, const char *opt, const char *arg, const struct arglist *);
#if !D4CUSTOM
extern void val_helpd3 (G *g, const char *opt, const char *arg, const struct arglist *);
#endif
extern void val_0arg (G *g, const char *opt, const char *arg, const struct arglist *);
extern void pval_0arg (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_uint (G *g, const char *opt, const char *arg, const struct arglist *);
extern void pval_uint (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_scale_uint (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_scale_uintd (G *g, const char *opt, const char *arg, const struct arglist *);
extern void pval_scale_uint (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_scale_pow2 (G *g, const char *opt, const char *arg, const struct arglist *);
extern void pval_scale_pow2 (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_char (G *g, const char *opt, const char *arg, const struct arglist *);
extern void pval_char (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_string (G *g, const char *opt, const char *arg, const struct arglist *);
extern void val_addr (G *g, const char *opt, const char *arg, const struct arglist *);

/*
 * custom functions -- help set up custom version of dineroIV.
 */
#if !D4CUSTOM
extern void custom_custom (G *g, const struct arglist *, FILE *);
extern void pcustom_0arg (G *g, const struct arglist *, FILE *);
extern void pcustom_uint (G *g, const struct arglist *, FILE *);
extern void pcustom_char (G *g, const struct arglist *, FILE *);
#endif

/*
 * summary functions -- produce summary of parameter setting
 */
extern void summary_0arg (const struct arglist *, FILE *);
extern void psummary_0arg (G *g, const struct arglist *, FILE *);
extern void summary_uint (G *g, const struct arglist *, FILE *);
extern void summary_uintd (G *g, const struct arglist *, FILE *);
extern void psummary_uint (G *g, const struct arglist *, FILE *);
extern void psummary_luint (G *g, const struct arglist *, FILE *);
extern void summary_char (G *g, const struct arglist *, FILE *);
extern void psummary_char (G *g, const struct arglist *, FILE *);
extern void summary_addr (G *g, const struct arglist *, FILE *);

/*
 * help functions -- support the -help arg
 */
extern void help_0arg (G *g, const struct arglist *);
extern void phelp_0arg (G *g, const struct arglist *);
extern void help_uint (G *g, const struct arglist *);
extern void help_scale_uintd (G *g, const struct arglist *);
extern void phelp_uint (G *g, const struct arglist *);
extern void phelp_scale_uint (G *g, const struct arglist *);
extern void phelp_scale_pow2 (G *g, const struct arglist *);
extern void help_char (G *g, const struct arglist *);
extern void phelp_char (G *g, const struct arglist *);
extern void help_string (G *g, const struct arglist *);
extern void help_addr (G *g, const struct arglist *);

/*
 * Set argument-related things up after seeing all args
 */
extern void verify_options (G *g);
extern void initialize_caches (G *g, d4cache **icachep, d4cache **dcachep);
extern void init_1cache (G *g, d4cache *, int, int);
