/*
 * Dinero IV
 * Written by Jan Edler and Mark D. Hill
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
 *
 * This file contains the startup, overall driving code, and
 * miscellaneous stuff needed for Dinero IV when running as a
 * self-contained simulator.
 * All the really hard stuff is in the callable Dinero IV subroutines,
 * which may be used independently of this program.
 *
 * $Header: /home/edler/dinero/d4/RCS/cmdmain.c,v 1.14 1998/02/06 20:58:11 edler Exp $
 */

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

//extern struct arglist args[];
//extern int nargs;

/* private prototypes for this file */
extern int do1arg (G *g, const char *, const char *);
extern void doargs (G *g, int, char **);
extern void summarize_caches (G *g, d4cache *, d4cache *);
extern void dostats (G *g);
extern void do1stats (d4cache *);
extern d4memref next_trace_item (G *g);
#if !D4CUSTOM
extern void customize_caches (G *g);
#endif


/*
 * Generic functions for handling command line arguments.
 * Most argument-specific knowledge is in cmdargs.c.
 */

/*
 * Internal function to handle one command line option.
 * Return number of command line args consumed.
 * (Doesn't really support more than 1 or 2 consumed.)
 */
int
do1arg (G *g, const char *opt, const char *arg)
{
	struct arglist *adesc;
	printf("%s %d ---------------------------\n", __FUNCTION__, __LINE__);
	for (adesc = g->args;  adesc->optstring != NULL;  adesc++) {
		printf("%s %d %s\n", __FUNCTION__, __LINE__, adesc->optstring);
		int eaten = adesc->match (opt, adesc);
		if (eaten > 0) {
			if (eaten > 1 && (arg == NULL || *arg == '-'))
				shorthelp (g, "\"%s\" option requires additional argument\n", opt);
			adesc->valf (opt, arg, adesc);
#if !D4CUSTOM
			if (adesc->customf == NULL) {
				g->cust_argv[g->cust_argc++] = opt;
				if (eaten>1)
					g->cust_argv[g->cust_argc++] = arg;
			}
#endif
			return eaten;
		}
	}
#if !D4CUSTOM
	/* does it look like a possible Dinero III option? */
	if (opt[0]=='-' && strchr ("uidbSarfpPwAQzZ", opt[1]) != NULL)
		shorthelp (g, "\"%s\" option not recognized for Dinero IV;\n"
			   "try \"%s -dineroIII\" for Dinero III --> IV option correspondence.\n",
			   opt, g->progname);
#endif
	//shorthelp (g, "\"%s\" option not recognized.\n", opt); FIXME disable to workaround a segfault
	return 0;	/* can't really get here, but some compilers get upset if we don't have a return value */
}


/*
 * Process all the command line args
 */
void
doargs (G *g, int argc, char **argv)
{
	struct arglist *adesc;
	char **v = argv+1;
	int x;

#if !D4CUSTOM
	g->cust_argv = malloc ((argc+1) * sizeof(argv[0]));
	if (g->cust_argv == NULL)
		die (g, "no memory to copy args for possible -custom\n");
#endif

	for (adesc = g->args;  adesc->optstring != NULL;  adesc++)
		if (g->optstringmax < (int)strlen(adesc->optstring) + adesc->pad)
			g->optstringmax = strlen(adesc->optstring) + adesc->pad;

	while (argc > 1) {
		const char *opt = v[0];
		const char *arg = (argc>1) ? v[1] : NULL;
		x = do1arg (g, opt, arg);
		v += x;
		argc -= x;
	}
	// verify_options();
}

/*
 * Get the level and idu portion of a -ln-idu prefix.
 * The return value is a pointer to the next character after the prefix,
 * or NULL if the prefix is not recognized.
 */
char *
level_idu (G *g, const char *opt, int *levelp, int *idup)
{
	int level;
	char *nextc;

	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);

	if (*opt++ != '-' || *opt++ != 'l')
		return NULL;	/* no initial -l */
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	if (*opt == '-' || *opt == '+')
		return NULL;	/* we don't accept a sign here */
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	level = strtol (opt, &nextc, 10);
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	if (nextc == opt)
		return NULL;	/* no digits */
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	if (level <= 0 || level > MAX_LEV)
		return NULL;	/* level out of range */
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	if (*nextc++ != '-')	/* missing - after level */
		return NULL;
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	switch (*nextc++) {
	default:	return NULL;	/* bad idu value */
	case 'u':	*idup = 0; break;
	case 'i':	*idup = 1; break;
	case 'd':	*idup = 2; break;
	}
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	*levelp = level - 1;
	if (level > g->maxlevel)
		g->maxlevel = level;
	printf("%s %d nextc=%s\n", __FUNCTION__, __LINE__, nextc);
	return nextc;
}


/*
 * Helper for scaled unsigned integer arguments
 */
unsigned int
argscale_uint (const char *arg, unsigned int *var)
{
	char *nextc;
	unsigned long x;

	errno = 0;
	if (!isdigit ((int)*arg))
		return 0;		/* no good: doesn't start with a number */
	x = strtoul (arg, &nextc, 10);
	if (nextc == arg)
		return 0;		/* no good: no chars consumed */
	if (x == ULONG_MAX && errno == ERANGE)
		return 0;		/* no good: overflow */
	switch (nextc[0]) {
	default:  return 0;		/* no good; some other trailing char */
	case 0:   break;		/* ok: no scale factor */
	case 'k':
	case 'K': if ((x>>(sizeof(x)*CHAR_BIT-10)) != 0)
			return 0;	/* no good: overflow */
		  x <<= 10;
		  break;
	case 'm':
	case 'M': if ((x>>(sizeof(x)*CHAR_BIT-20)) != 0)
			return 0;	/* no good: overflow */
		  x <<= 20;
		  break;
	case 'g':
	case 'G': if ((x>>(sizeof(x)*CHAR_BIT-30)) != 0)
			  return 0;	/* no good: overflow */
		  x <<= 30;
		  break;
	}
	if (nextc[0] != 0 && nextc[1] != 0)
		return 0;		/* no good: trailing junk */
	*var = x;
	return 1;
}


/*
 * Helper for scaled unsigned integer arguments, but using double for extra range
 * We go through various difficulties just to allow use of strtod
 * while still preventing non-integer use.
 * XXX We should probably use autoconf to help us use long or long long
 * or whatever, if sufficient.
 */
double
argscale_uintd (const char *arg, double *var)
{
	char *nextc;
	double x, ipart;

	errno = 0;
	x = strtod (arg, &nextc);
	if (nextc == arg)
		return 0;		/* no good: no chars consumed */
	if (x == HUGE_VAL && errno == ERANGE)
		return 0;		/* no good: overflow */
	/* make sure value is an integer */
	switch (nextc[0]) {
	default:  return 0;		/* no good; some other trailing char */
	case 0:   break;		/* ok: no scale factor */
	case 'k':
	case 'K': 
		  x *= (1<<10);
		  break;
	case 'm':
	case 'M': 
		  x *= (1<<20);
		  break;
	case 'g':
	case 'G': 
		  x *= (1<<30);
		  break;
	}
	if (nextc[0] != 0 && nextc[1] != 0)
		return 0;		/* no good: trailing junk */
	/* make sure number was unsigned integer; no decimal pt, exponent, or sign */
	for (;  arg < nextc;  arg++)
		if (!isdigit ((int)*arg))
			return 0;	/* no good: non-digits */
	/* make sure we don't have a fractional part due to scaling */
	if (modf (x, &ipart) != 0)
		return 0;		/* no good: fraction != 0 */
	*var = x;
	return 1;
}


/*
 * Recognize an option with no args
 */
int
match_0arg (G *g, const char *opt, const struct arglist *adesc)
{
	//printf("%s %d %s\n", __FUNCTION__, __LINE__, opt);
	return strcmp (opt, adesc->optstring) == 0;
}


/*
 * Recognize an option with no args and the -ln-idu prefix
 */
int
pmatch_0arg (G *g, const char *opt, const struct arglist *adesc)
{
	printf("%s %d g=%x\n", __FUNCTION__, __LINE__, g);
	int level;
	int idu;
	const char *nextc = level_idu (g, opt, &level, &idu);
	if (nextc == NULL)
		return 0;	/* not recognized */
	return 1 * (strcmp (nextc, adesc->optstring) == 0);
}


/*
 * Recognize an option with 1 arg
 */
int
match_1arg (G *g, const char *opt, const struct arglist *adesc)
{
	printf("%s %d %s vs %s\n", __FUNCTION__, __LINE__, opt, adesc->optstring);
	return 2 * (strcmp (opt, adesc->optstring) == 0);
}


/*
 * Recognize an option with 1 arg and the -ln-idu prefix
 */
int
pmatch_1arg (G *g, const char *opt, const struct arglist *adesc)
{
	printf("%s %d opt=%s\n", __FUNCTION__, __LINE__, opt);
	int level;
	int idu;
	const char *nextc = level_idu (g, opt, &level, &idu);
	if (nextc == NULL)
		return 0;	/* not recognized */
	printf("%s %d\n", __FUNCTION__, __LINE__);
	return 2 * (strcmp (nextc, adesc->optstring) == 0);
}


#if D4CUSTOM
/*
 * Recognize no option; used as a bogus match function for options
 * which have been customized.
 */
int
match_bogus (const char *opt, const struct arglist *adesc)
{
        return 0;
}
#endif


/*
 * Produce help message in response to -help
 */
void
val_help (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	int i;

	printf ("Usage: %s [options]\nValid options:\n", g->progname);
	for (i = 0;  i < g->nargs;  i++) {
		if (g->args[i].optstring != NULL && g->args[i].help != NULL) {
			putchar (' ');
			g->args[i].help (&g->args[i]);
			if (g->args[i].defstr != NULL)
				printf (" (default %s)", g->args[i].defstr);
			putchar ('\n');
		}
	}
	printf ("Key:\n");
	printf (" U unsigned decimal integer\n");
	printf (" S like U but with optional [kKmMgG] scaling suffix\n");
	printf (" P like S but must be a power of 2\n");
	printf (" C single character\n");
	printf (" A hexadecimal address\n");
	printf (" F string\n");
	printf (" N cache level (1 <= N <= %d)\n", MAX_LEV);
	printf (" T cache type (u=unified, i=instruction, d=data)\n");
	exit(0);
}


/*
 * Produce help message in response to -copyright
 */
void
val_helpcr (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	printf ("Dinero IV is copyrighted software\n");
	printf ("\n");
	printf ("Copyright (C) 1997 NEC Research Institute, Inc. and Mark D. Hill.\n");
	printf ("All rights reserved.\n");
	printf ("Copyright (C) 1985, 1989 Mark D. Hill.  All rights reserved.\n");
	printf ("\n");
	printf ("Permission to use, copy, modify, and distribute this software and\n");
	printf ("its associated documentation for non-commercial purposes is hereby\n");
	printf ("granted (for commercial purposes see below), provided that the above\n");
	printf ("copyright notice appears in all copies, derivative works or modified\n");
	printf ("versions of the software and any portions thereof, and that both the\n");
	printf ("copyright notice and this permission notice appear in the documentation.\n");
	printf ("NEC Research Institute Inc. and Mark D. Hill shall be given a copy of\n");
	printf ("any such derivative work or modified version of the software and NEC\n");
	printf ("Research Institute Inc.  and any of its affiliated companies (collectively\n");
	printf ("referred to as NECI) and Mark D. Hill shall be granted permission to use,\n");
	printf ("copy, modify, and distribute the software for internal use and research.\n");
	printf ("The name of NEC Research Institute Inc. and its affiliated companies\n");
	printf ("shall not be used in advertising or publicity related to the distribution\n");
	printf ("of the software, without the prior written consent of NECI.  All copies,\n");
	printf ("derivative works, or modified versions of the software shall be exported\n");
	printf ("or reexported in accordance with applicable laws and regulations relating\n");
	printf ("to export control.  This software is experimental.  NECI and Mark D. Hill\n");
	printf ("make no representations regarding the suitability of this software for\n");
	printf ("any purpose and neither NECI nor Mark D. Hill will support the software.\n");
	printf ("\n");
	printf ("Use of this software for commercial purposes is also possible, but only\n");
	printf ("if, in addition to the above requirements for non-commercial use, written\n");
	printf ("permission for such use is obtained by the commercial user from NECI or\n");
	printf ("Mark D. Hill prior to the fabrication and distribution of the software.\n");
	printf ("\n");
	printf ("THE SOFTWARE IS PROVIDED AS IS.  NECI AND MARK D. HILL DO NOT MAKE\n");
	printf ("ANY WARRANTEES EITHER EXPRESS OR IMPLIED WITH REGARD TO THE SOFTWARE.\n");
	printf ("NECI AND MARK D. HILL ALSO DISCLAIM ANY WARRANTY THAT THE SOFTWARE IS\n");
	printf ("FREE OF INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OF OTHERS.\n");
	printf ("NO OTHER LICENSE EXPRESS OR IMPLIED IS HEREBY GRANTED.  NECI AND MARK\n");
	printf ("D. HILL SHALL NOT BE LIABLE FOR ANY DAMAGES, INCLUDING GENERAL, SPECIAL,\n");
	printf ("INCIDENTAL, OR CONSEQUENTIAL DAMAGES, ARISING OUT OF THE USE OR INABILITY\n");
	printf ("TO USE THE SOFTWARE.\n");
	exit(0);
}


/*
 * Produce help message in response to -contact
 */
void
val_helpw (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	printf ("Dinero IV was written by Jan Edler and Mark D. Hill.\n");
	printf ("\n");
	printf ("    Dr. Jan Edler                    Prof. Mark D. Hill\n");
	printf ("    NEC Research Institute           Computer Sciences Dept.\n");
	printf ("    4 Independence Way               Univ. of Wisconsin\n");
	printf ("    Princeton, NJ 08540              Madison, WI 53706\n");
	printf ("    edler@research.nj.nec.com        markhill@cs.wisc.edu\n");
	printf ("    edler@acm.org\n");
	printf ("    edler@computer.org\n");
	printf ("\n");
	printf ("The latest version of Dinero IV can be obtained\n");
	printf ("from the following places:\n");
	printf ("       http://www.neci.nj.nec.com/homepages/edler/d4\n");
	printf ("       http://www.cs.wisc.edu/~markhill/DineroIV\n");
	printf ("       ftp://ftp.nj.nec.com/pub/edler/d4\n");
	printf ("       ftp://ftp.cs.wisc.edu/markhill/DineroIV\n");
	printf ("Copyright terms are explained by the -copyright option.\n");
	exit(0);
}


#if !D4CUSTOM
/*
 * Explain DineroIII->DineroIV option mappings
 */
void
val_helpd3 (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	printf ("Summary of DineroIV replacements for DineroIII options:\n\n");
	printf ("DineroIII    DineroIV\n");
	printf ("-u           -l1-usize\n");
	printf ("-i           -l1-isize\n");
	printf ("-d           -l1-dsize\n");
	printf ("-b           -l1-ubsize, -l1-ibsize, or -l1-dbsize\n");
	printf ("-S           -l1-usbsize, -l1-isbsize, or -l1-dsbsize\n");
	printf ("-a           -l1-uassoc, -l1-iassoc, or -l1-dassoc\n");
	printf ("-r           -l1-urepl, -l1-irepl, or -l1-drepl\n");
	printf ("-f           -l1-ufetch, -l1-ifetch, or -l1-dfetch\n");
	printf ("-p           -l1-upfdist, -l1-ipfdist, or -l1-dpfdist\n");
	printf ("-P           -l1-upfabort, -l1-ipfabort, or -l1-dpfabort\n");
	printf ("-w           -l1-uwback or -l1-dwback\n");
	printf ("-A           -l1-uwalloc or -l1-dwalloc\n");
	printf ("-Q           -flushcount\n");
	printf ("-z           -maxcount\n");
	printf ("-Z           -skipcount\n");
	printf ("\n");
	printf ("DineroIII input format: -informat d\n");
	exit(0);
}
#endif


/*
 * Handle an option with no args (i.e., a boolean option).
 */
void
val_0arg (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	*(int *) adesc->var = 1;
}


/*
 * Handle an option with no args (i.e., a boolean option) and the -ln-idu-prefix.
 */
void
pval_0arg (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	int (*var)[3][MAX_LEV] = adesc->var;
	int level;
	int idu;

	(void) level_idu (g, opt, &level, &idu);
	(*var)[idu][level] = 1;
}


/*
 * Handle unsigned integer arg.
 */
void
val_uint (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	char *nextc;

	*(unsigned int *)adesc->var = strtoul (arg, &nextc, 10);
	if (*nextc != 0)
		shorthelp (g, "bad option: %s %s\n", opt, arg);
}


/*
 * Handle -ln-idu-stuff with unsigned integer arg.
 * Note the match function has already verified the form,
 * so level_idu() can't return NULL.
 */
void
pval_uint (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	unsigned int (*var)[3][MAX_LEV] = adesc->var;
	unsigned int argui;
	int level;
	int idu;
	char *nextc;

	(void) level_idu (g, opt, &level, &idu);
	argui = strtoul (arg, &nextc, 10);
	if (*nextc != 0)
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	(*var)[idu][level] = argui;
}


/*
 * Handle unsigned integer arg with optional scaling (k,m,g, etc).
 */
void
val_scale_uint (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	if (!argscale_uint (arg, adesc->var))
		shorthelp (g, "bad option: %s %s\n", opt, arg);
}


/*
 * Handle unsigned integer arg with optional scaling (k,m,g, etc).
 * Here we use double for extra integer range.
 */
void
val_scale_uintd (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	if (!argscale_uintd (arg, adesc->var))
		shorthelp (g, "bad option: %s %s\n", opt, arg);
}


/*
 * Handle -ln-idu-stuff with unsigned integer arg with
 * optional scaling (k,m,g, etc).
 * Note the match function has already verified the form,
 * so level_idu() can't return NULL.
 */
void
pval_scale_uint (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	unsigned int (*var)[3][MAX_LEV] = adesc->var;
	unsigned int argui;
	int level;
	int idu;

	(void) level_idu (g, opt, &level, &idu);
	if (!argscale_uint (arg, &argui))
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	(*var)[idu][level] = argui;
}


/*
 * Handle unsigned integer arg restricted to a power of 2 and
 * with optional scaling (k,m,g, etc).
 */
void
val_scale_pow2 (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	unsigned int *ui = adesc->var;

	if (!argscale_uint (arg, ui))
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	if (*ui == 0 || (*ui & (*ui-1)) != 0)
		shorthelp (g, "option %s arg must be power of 2\n", opt);
}


/*
 * Handle -ln-idu-stuff with unsigned integer arg restricted to
 * a power of 2 and with optional scaling (k,m,g, etc).
 * Note the match function has already verified the form,
 * so level_idu() can't return NULL.
 */
void
pval_scale_pow2 (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	unsigned int (*var)[3][MAX_LEV] = adesc->var;
	unsigned int argui;
	int level;
	int idu;

	(void) level_idu (g, opt, &level, &idu);
	if (!argscale_uint (arg, &argui))
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	if (argui == 0 || (argui & (argui-1)) != 0)
		shorthelp (g, "option %s arg must be power of 2\n", opt);
	(*var)[idu][level] = argui;
}


/*
 * Handle an option with a single character as arg
 */
void
val_char (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	int *var = adesc->var;

	if (strlen (arg) != 1)
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	*var = *arg;
}


/*
 * Handle an option with level/idu prefix and a single character as arg
 */
void
pval_char (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	int (*var)[3][MAX_LEV] = adesc->var;
	int level;
	int idu;

	(void) level_idu (g, opt, &level, &idu);
	if (strlen (arg) != 1)
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	(*var)[idu][level] = *arg;
}


/*
 * Handle an option with a string as arg
 */
void
val_string (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	*(const char **)adesc->var = arg;
}


/*
 * Handle an option with a hexadecimal address as arg.
 */
void
val_addr (G *g, const char *opt, const char *arg, const struct arglist *adesc)
{
	long *var = adesc->var;
	long argl;
	char *nextc;

	argl = strtoul (arg, &nextc, 16);
	if (*nextc != 0)
		shorthelp (g, "bad option: %s %s\n", opt, arg);
	*var = argl;
}


#if !D4CUSTOM	/* a customized program can't be further customized */
/*
 * Do nothing.  This is just to provide a non-null function for -custom
 * so do1arg will not think this option should be passed on to the
 * custom program itself.
 */
void
custom_custom (G *g, const struct arglist *adesc, FILE *hfile)
{
}


/*
 * Output an array initializer for the level/idu array whose name is given
 * in the customstring field of the arglist structure,
 * when the values in question are boolean.
 */
void
pcustom_0arg (G *g, const struct arglist *adesc, FILE *hfile)
{
	int i, j;
	int (*var)[3][MAX_LEV] = adesc->var;

	fprintf (hfile, "int %s[3][MAX_LEV] = {\n",
		 adesc->customstring);
	for (i = 0;  i < 3;  i++) {
		fprintf (hfile, " { ");
		for (j = 0;  j < g->maxlevel;  j++)
			fprintf (hfile, "%d%s ", (*var)[i][j],
				 j<g->maxlevel-1 ? "," : "");
		fprintf (hfile, "}%s\n", i<2 ? "," : "");
	}
	fprintf (hfile, "};\n");
}


/*
 * Output an array initializer for the level/idu array whose name is given
 * in the customstring field of the arglist structure,
 * when the values in question are unsigned ints.
 */
void
pcustom_uint (G *g, const struct arglist *adesc, FILE *hfile)
{
	int i, j;
	unsigned int (*var)[3][MAX_LEV] = adesc->var;

	fprintf (hfile, "unsigned int %s[3][MAX_LEV] = {\n",
		 adesc->customstring);
	for (i = 0;  i < 3;  i++) {
		fprintf (hfile, " { ");
		for (j = 0;  j < g->maxlevel;  j++)
			fprintf (hfile, "%u%s ", (*var)[i][j],
				 j<g->maxlevel-1 ? "," : "");
		fprintf (hfile, "}%s\n", i<2 ? "," : "");
	}
	fprintf (hfile, "};\n");
}


/*
 * Output an array initializer for the level/idu array whose name is given
 * in the customstring field of the arglist structure,
 * when the values in question are chars.
 */
void
pcustom_char (G *g, const struct arglist *adesc, FILE *hfile)
{
	int i, j;
	int (*var)[3][MAX_LEV] = adesc->var;

	fprintf (hfile, "int %s[3][MAX_LEV] = {\n",
		 adesc->customstring);
	for (i = 0;  i < 3;  i++) {
		fprintf (hfile, " { ");
		for (j = 0;  j < g->maxlevel;  j++)
			fprintf (hfile, "%d%s ", (*var)[i][j],
				 j<g->maxlevel-1 ? "," : "");
		fprintf (hfile, "}%s\n", i<2 ? "," : "");
	}
	fprintf (hfile, "};\n");
}
#endif	/* !D4CUSTOM */


/*
 * Produce a summary line for parameters with no arg (i.e., boolean parameters).
 */
void
summary_0arg (const struct arglist *adesc, FILE *f)
{
	if (*(int *)adesc->var != 0)
		fprintf (f, "%s\n", adesc->optstring);
}


/*
 * Produce a summary line for parameters with level/idu prefix but no arg.
 */
void
psummary_0arg (G *g, const struct arglist *adesc, FILE *f)
{
	int idu, lev;
	int (*var)[3][MAX_LEV] = adesc->var;

	for (idu = 0;  idu < 3;  idu++) {
		for (lev = 0;  lev <= g->maxlevel;  lev++) {
			if ((*var)[idu][lev] != 0)
				fprintf (f, "-l%d-%c%s\n", lev+1,
					 idu==0?'u':(idu==1?'i':'d'),
					 adesc->optstring);
		}
	}
}


/*
 * Produce a summary line for parameters with typical unsigned integer value.
 */
void
summary_uint (G *g, const struct arglist *adesc, FILE *f)
{
	fprintf (f, "%s %u\n", adesc->optstring, *(unsigned int *)adesc->var);
}


/*
 * Produce a summary line for parameters with typical unsigned integer value,
 * handled as a double for extended range.
 */
void
summary_uintd (G *g, const struct arglist *adesc, FILE *f)
{
	fprintf (f, "%s %.0f\n", adesc->optstring, *(double *)adesc->var);
}


/*
 * Produce a summary line for parameters with level/idu prefix and typical
 * unsigned integer value.
 */
void
psummary_uint (G *g, const struct arglist *adesc, FILE *f)
{
	int idu, lev;
	unsigned int (*var)[3][MAX_LEV] = adesc->var;

	for (idu = 0;  idu < 3;  idu++) {
		for (lev = 0;  lev <= g->maxlevel;  lev++) {
			if ((*var)[idu][lev] != 0) {
				fprintf (f, "-l%d-%c%s %u\n", lev+1,
					 idu==0?'u':(idu==1?'i':'d'),
					 adesc->optstring, (*var)[idu][lev]);
			}
		}
	}
}


/*
 * Produce a summary line for parameters with level/idu prefix and typical
 * unsigned power-of-2 integer value, remembered as its log.
 */
void
psummary_luint (G *g, const struct arglist *adesc, FILE *f)
{
#if 0
	int idu, lev;
	unsigned int (*var)[3][MAX_LEV] = adesc->var;

	for (idu = 0;  idu < 3;  idu++) {
		for (lev = 0;  lev <= g->maxlevel;  lev++) {
			if ((*var)[idu][lev] != 0) {
				fprintf (f, "-l%d-%c%s %u\n", lev+1,
					 idu==0?'u':(idu==1?'i':'d'),
					 adesc->optstring,
					 (*var)[idu][lev]);
			}
		}
	}
#endif	//0
}


/*
 * Produce a summary line for parameters with typical single char argument.
 */
void
summary_char (G *g, const struct arglist *adesc, FILE *f)
{
#if 0
	fprintf (f, "%s %c\n", adesc->optstring, *(int *)adesc->var);
#endif	//0
}


/*
 * Produce a summary line for parameters with level/idu prefix and typical
 * single char value.
 */
void
psummary_char (G *g, const struct arglist *adesc, FILE *f)
{
#if 0
	int idu, lev;
	int (*var)[3][MAX_LEV] = adesc->var;

	for (idu = 0;  idu < 3;  idu++) {
		for (lev = 0;  lev <= g->maxlevel;  lev++) {
			if ((*var)[idu][lev] != 0) {
				fprintf (f, "-l%d-%c%s %c\n", lev+1,
					 idu==0?'u':(idu==1?'i':'d'),
					 adesc->optstring,
					 (*var)[idu][lev]);
			}
		}
	}
#endif	//0
}


/*
 * Produce a summary line for parameters with typical hexadecimal address
 * as argument.
 */
void
summary_addr (G *g, const struct arglist *adesc, FILE *f)
{
#if 0
	fprintf (f, "%s 0x%lx\n", adesc->optstring, *(long *)adesc->var);
#endif	//0
}


/*
 * Produce a help line for a possible command line option taking no args.
 */
void
help_0arg (G *g, const struct arglist *adesc)
{
#if 0
	printf ("%-*s %s", g->optstringmax, adesc->optstring, adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * level/idu prefix and no args.
 * We don't bother trying to show the default values; there are too many.
 */
void
phelp_0arg (G *g, const struct arglist *adesc)
{
#if 0
        printf ("-lN-T%-*s %s", g->optstringmax-adesc->pad, adesc->optstring, adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option taking
 * an unsigned int value.
 */
void
help_uint (G *g, const struct arglist *adesc)
{
#if 0
	printf ("%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "U",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option taking
 * a scaled unsigned int value, but using double for extra range.
 */
void
help_scale_uintd (G *g, const struct arglist *adesc)
{
#if 0
	printf ("%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "U",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * level/idu prefix and unsigned int value.
 * We don't bother trying to show the default values; there are too many.
 */
void
phelp_uint (G *g, const struct arglist *adesc)
{
#if 0
	printf ("-lN-T%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "U",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * level/idu prefix and scaled unsigned int value.
 * We don't bother trying to show the default values; there are too many.
 */
void
phelp_scale_uint (G *g, const struct arglist *adesc)
{
#if 0
	printf ("-lN-T%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "S",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * level/idu prefix and scaled power-of-2 unsigned int value.
 * We don't bother trying to show the default values; there are too many.
 */
void
phelp_scale_pow2 (G *g, const struct arglist *adesc)
{
#if 0
	printf ("-lN-T%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "P",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * a single char value.
 */
void
help_char (G *g, const struct arglist *adesc)
{
#if 0
	printf ("%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "C",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * level/idu prefix and single char value.
 * We don't bother trying to show the default values; there are too many.
 */
void
phelp_char (G *g, const struct arglist *adesc)
{
#if 0
	printf ("-lN-T%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "C",
		adesc->helpstring);
#endif	//0
}


/*
 * Produce a help line for a possible command line option taking a string.
 */
void
help_string (G *g, const struct arglist *adesc)
{
#if 0
	printf ("%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "F",
		adesc->helpstring);
	if (*(char **)adesc->var != NULL)
		printf (" (\"%s\")", *(char **)adesc->var);
#endif	//0
}


/*
 * Produce a help line for a possible command line option with
 * a hexidecimal address value.
 */
void
help_addr (G *g, const struct arglist *adesc)
{
#if 0
	printf ("%s %-*s %s", adesc->optstring,
		g->optstringmax-(int)strlen(adesc->optstring)-adesc->pad+1, "A",
		adesc->helpstring);
	if (*(long *)adesc->var != 0)
		printf (" (0x%lx)", *(long *)adesc->var);
#endif	//0
}


/*
 * Print info about how the caches are set up
 */
void
summarize_caches (G *g, d4cache *ci, d4cache *cd)
{
	struct arglist *adesc;

	printf ("\n---Summary of options "
		"(-help option gives usage information).\n\n");

#if 0
	for (adesc = args;  adesc->optstring != NULL;  adesc++)
		if (adesc->sumf != (void (*)())NULL)
			adesc->sumf (adesc, stdout);
#endif	//0
}


/*
 * Print out the stuff the user really wants
 */
void
dostats(G *g)
{
	int lev;
	int i;
	for (lev = 0;  lev < g->maxlevel;  lev++) {
		if (g->stat_idcombine && g->levcache[0][lev] == NULL) {
			d4cache cc;	/* a bogus cache structure */
			char ccname[30];

			cc.name = ccname;
			sprintf (cc.name, "l%d-I/Dcaches", lev+1);
			cc.prefetchf = NULL;
			if (g->levcache[1][lev]->prefetchf==d4prefetch_none && g->levcache[2][lev]->prefetchf==d4prefetch_none)
				cc.prefetchf = d4prefetch_none; /* controls prefetch printout stats */

			/* add the i & d stats into the new bogus structure */
			for (i = 0;  i < 2 * D4NUMACCESSTYPES;  i++) {
				cc.fetch[i]          = g->levcache[1][lev]->fetch[i]          + g->levcache[2][lev]->fetch[i];
				cc.miss[i]           = g->levcache[1][lev]->miss[i]           + g->levcache[2][lev]->miss[i];
				cc.blockmiss[i]      = g->levcache[1][lev]->blockmiss[i]      + g->levcache[2][lev]->blockmiss[i];
				cc.comp_miss[i]      = g->levcache[1][lev]->comp_miss[i]      + g->levcache[2][lev]->comp_miss[i];
				cc.comp_blockmiss[i] = g->levcache[1][lev]->comp_blockmiss[i] + g->levcache[2][lev]->comp_blockmiss[i];
				cc.cap_miss[i]       = g->levcache[1][lev]->cap_miss[i]       + g->levcache[2][lev]->cap_miss[i];
				cc.cap_blockmiss[i]  = g->levcache[1][lev]->cap_blockmiss[i]  + g->levcache[2][lev]->cap_blockmiss[i];
				cc.conf_miss[i]      = g->levcache[1][lev]->conf_miss[i]      + g->levcache[2][lev]->conf_miss[i];
				cc.conf_blockmiss[i] = g->levcache[1][lev]->conf_blockmiss[i] + g->levcache[2][lev]->conf_blockmiss[i];
			}
			cc.multiblock    = g->levcache[1][lev]->multiblock    + g->levcache[2][lev]->multiblock;
			cc.bytes_read    = g->levcache[1][lev]->bytes_read    + g->levcache[2][lev]->bytes_read;
			cc.bytes_written = g->levcache[1][lev]->bytes_written + g->levcache[2][lev]->bytes_written;

			cc.flags = g->levcache[1][lev]->flags | g->levcache[2][lev]->flags; /* get D4F_CCC */

			/*
			 * block and subblock size should match;
			 * should be checked at startup time
			 */
			cc.lg2subblocksize = g->levcache[1][lev]->lg2subblocksize;
			cc.lg2blocksize = g->levcache[1][lev]->lg2blocksize;

			do1stats (&cc);
		}
		else {
			if (g->levcache[0][lev] != NULL)
				do1stats (g->levcache[0][lev]);
			if (g->levcache[1][lev] != NULL)
				do1stats (g->levcache[1][lev]);
			if (g->levcache[2][lev] != NULL)
				do1stats (g->levcache[2][lev]);
		}
	}
}


#define NONZERO(i) (((i)==0.0) ? 1.0 : (double)(i)) /* avoid divide-by-zero exception */
/*
 * Print stats for 1 cache
 */
void
do1stats (d4cache *c)
{
	double	demand_fetch_data,
		demand_fetch_alltype;
	double	prefetch_fetch_data,
		prefetch_fetch_alltype;

	double	demand_data,
		demand_alltype;
	double	prefetch_data,
		prefetch_alltype;

	double	demand_comp_data,
		demand_comp_alltype;
	double	demand_cap_data,
		demand_cap_alltype;
	double	demand_conf_data,
		demand_conf_alltype;

	double	floatnum;

	/* Used in bus traffic calculations even if no prefetching. */
	prefetch_fetch_alltype = 0;

	/*
	 * Print Header
	 */
	printf(	"%s\n", c->name);
	printf(	" Metrics		      Total	      Instrn	       Data	       Read	      Write	       Misc\n");
	printf(	" -----------------	      ------	      ------	      ------	      ------	      ------	      ------\n");

	/*
	 * Print Fetch Numbers
	 */
	demand_fetch_data = c->fetch[D4XMISC] 
			  + c->fetch[D4XREAD]
		      	  + c->fetch[D4XWRITE];
	demand_fetch_alltype = demand_fetch_data 
			  + c->fetch[D4XINSTRN];

	printf(	" Demand Fetches		%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
		demand_fetch_alltype,
		c->fetch[D4XINSTRN],
		demand_fetch_data,
		c->fetch[D4XREAD],
		c->fetch[D4XWRITE],
		c->fetch[D4XMISC]);

	floatnum = NONZERO(demand_fetch_alltype);

	printf(	"  Fraction of total	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
		demand_fetch_alltype / floatnum,
		c->fetch[D4XINSTRN] / floatnum,
		demand_fetch_data / floatnum,
		c->fetch[D4XREAD] / floatnum,
		c->fetch[D4XWRITE] / floatnum,
		c->fetch[D4XMISC] / floatnum);

	/*
	 * Prefetching?
	 */
	prefetch_fetch_data = c->fetch[D4PREFETCH+D4XMISC] 
			  + c->fetch[D4PREFETCH+D4XREAD]
		      	  + c->fetch[D4PREFETCH+D4XWRITE];
	if (c->prefetchf != d4prefetch_none) {
		prefetch_fetch_alltype = prefetch_fetch_data 
				  + c->fetch[D4PREFETCH+D4XINSTRN];

		printf(	" Prefetch Fetches	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			prefetch_fetch_alltype,
			c->fetch[D4PREFETCH+D4XINSTRN],
			prefetch_fetch_data,
			c->fetch[D4PREFETCH+D4XREAD],
			c->fetch[D4PREFETCH+D4XWRITE],
			c->fetch[D4PREFETCH+D4XMISC]);

		floatnum = NONZERO(prefetch_fetch_alltype);

		printf(	"  Fraction		%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			prefetch_fetch_alltype / floatnum,
			c->fetch[D4PREFETCH+D4XINSTRN] / floatnum,
			prefetch_fetch_data / floatnum,
			c->fetch[D4PREFETCH+D4XREAD] / floatnum,
			c->fetch[D4PREFETCH+D4XWRITE] / floatnum,
			c->fetch[D4PREFETCH+D4XMISC] / floatnum);

		printf(	" Total Fetches		%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			demand_fetch_alltype + prefetch_fetch_alltype,
			c->fetch[D4XINSTRN] + c->fetch[D4PREFETCH+D4XINSTRN],
			demand_fetch_data + prefetch_fetch_data,
			c->fetch[D4XREAD] + c->fetch[D4PREFETCH+D4XREAD],
			c->fetch[D4XWRITE] + c->fetch[D4PREFETCH+D4XWRITE],
			c->fetch[D4XMISC] + c->fetch[D4PREFETCH+D4XMISC]);

		floatnum = NONZERO(demand_fetch_alltype + prefetch_fetch_alltype);

		printf(	"  Fraction		%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			(demand_fetch_alltype + prefetch_fetch_alltype) / floatnum,
			(c->fetch[D4XINSTRN] + c->fetch[D4PREFETCH+D4XINSTRN]) / floatnum,
			(demand_fetch_data + prefetch_fetch_data) / floatnum,
			(c->fetch[D4XREAD] + c->fetch[D4PREFETCH+D4XREAD]) / floatnum,
			(c->fetch[D4XWRITE] + c->fetch[D4PREFETCH+D4XWRITE]) / floatnum,
			(c->fetch[D4XMISC] + c->fetch[D4PREFETCH+D4XMISC]) / floatnum);

	} /* End of prefetching. */
	printf("\n");

	/*
	 * End of Fetch Numbers
	 */

	/*
	 * Print Miss Numbers
	 */
	demand_data = c->miss[D4XMISC] 
			  + c->miss[D4XREAD]
		      	  + c->miss[D4XWRITE];
	demand_alltype = demand_data 
			  + c->miss[D4XINSTRN];

	printf(	" Demand Misses		%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
		demand_alltype,
		c->miss[D4XINSTRN],
		demand_data,
		c->miss[D4XREAD],
		c->miss[D4XWRITE],
		c->miss[D4XMISC]);


	printf(	"  Demand miss rate	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
		demand_alltype / NONZERO(demand_fetch_alltype),
		c->miss[D4XINSTRN] / NONZERO(c->fetch[D4XINSTRN]),
		demand_data / NONZERO(demand_fetch_data),
		c->miss[D4XREAD] / NONZERO(c->fetch[D4XREAD]),
		c->miss[D4XWRITE] / NONZERO(c->fetch[D4XWRITE]),
		c->miss[D4XMISC] / NONZERO(c->fetch[D4XMISC]));

	if (c->flags & D4F_CCC) {
		demand_comp_data = c->comp_miss[D4XMISC] 
			  + c->comp_miss[D4XREAD]
		      	  + c->comp_miss[D4XWRITE];
		demand_comp_alltype = demand_comp_data 
			  + c->comp_miss[D4XINSTRN];
		demand_cap_data = c->cap_miss[D4XMISC] 
			  + c->cap_miss[D4XREAD]
		      	  + c->cap_miss[D4XWRITE];
		demand_cap_alltype = demand_cap_data 
			  + c->cap_miss[D4XINSTRN];
		demand_conf_data = c->conf_miss[D4XMISC] 
			  + c->conf_miss[D4XREAD]
		      	  + c->conf_miss[D4XWRITE];
		demand_conf_alltype = demand_conf_data 
			  + c->conf_miss[D4XINSTRN];

		printf(	"   Compulsory misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			demand_comp_alltype,
			c->comp_miss[D4XINSTRN],
			demand_comp_data,
			c->comp_miss[D4XREAD],
			c->comp_miss[D4XWRITE],
			c->comp_miss[D4XMISC]);
	    
		printf(	"   Capacity misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			demand_cap_alltype,
			c->cap_miss[D4XINSTRN],
			demand_cap_data,
			c->cap_miss[D4XREAD],
			c->cap_miss[D4XWRITE],
			c->cap_miss[D4XMISC]);
	    
		printf(	"   Conflict misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			demand_conf_alltype,
			c->conf_miss[D4XINSTRN],
			demand_conf_data,
			c->conf_miss[D4XREAD],
			c->conf_miss[D4XWRITE],
			c->conf_miss[D4XMISC]);

		printf(	"   Compulsory fraction	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			demand_comp_alltype / NONZERO(demand_alltype),
			c->comp_miss[D4XINSTRN] / NONZERO(c->miss[D4XINSTRN]),
			demand_comp_data / NONZERO(demand_data),
			c->comp_miss[D4XREAD] / NONZERO(c->miss[D4XREAD]),
			c->comp_miss[D4XWRITE] / NONZERO(c->miss[D4XWRITE]),
			c->comp_miss[D4XMISC] / NONZERO(c->miss[D4XMISC]));
	    
		printf(	"   Capacity fraction	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			demand_cap_alltype / NONZERO(demand_alltype),
			c->cap_miss[D4XINSTRN]  / NONZERO(c->miss[D4XINSTRN]),
			demand_cap_data / NONZERO(demand_data),
			c->cap_miss[D4XREAD] / NONZERO(c->miss[D4XREAD]),
			c->cap_miss[D4XWRITE] / NONZERO(c->miss[D4XWRITE]),
			c->cap_miss[D4XMISC] / NONZERO(c->miss[D4XMISC]));
	    
		printf(	"   Conflict fraction	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			demand_conf_alltype / NONZERO(demand_alltype),
			c->conf_miss[D4XINSTRN] / NONZERO(c->miss[D4XINSTRN]),
			demand_conf_data / NONZERO(demand_data),
			c->conf_miss[D4XREAD] / NONZERO(c->miss[D4XREAD]),
			c->conf_miss[D4XWRITE] / NONZERO(c->miss[D4XWRITE]),
			c->conf_miss[D4XMISC] / NONZERO(c->miss[D4XMISC]));
	}

	/*
	 * Prefetch misses?
	 */
	if (c->prefetchf != d4prefetch_none) {
		prefetch_data = c->miss[D4PREFETCH+D4XMISC] 
				  + c->miss[D4PREFETCH+D4XREAD]
			      	  + c->miss[D4PREFETCH+D4XWRITE];
		prefetch_alltype = prefetch_data 
				  + c->miss[D4PREFETCH+D4XINSTRN];

		printf(	" Prefetch Misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			prefetch_alltype,
			c->miss[D4PREFETCH+D4XINSTRN],
			prefetch_data,
			c->miss[D4PREFETCH+D4XREAD],
			c->miss[D4PREFETCH+D4XWRITE],
			c->miss[D4PREFETCH+D4XMISC]);

		printf(	"  PF miss rate		%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			prefetch_alltype / NONZERO(prefetch_fetch_alltype),
			c->miss[D4PREFETCH+D4XINSTRN] / NONZERO(c->fetch[D4PREFETCH+D4XINSTRN]),
			prefetch_data / NONZERO(prefetch_fetch_data),
			c->miss[D4PREFETCH+D4XREAD] / NONZERO(c->fetch[D4PREFETCH+D4XREAD]),
			c->miss[D4PREFETCH+D4XWRITE] / NONZERO(c->fetch[D4PREFETCH+D4XWRITE]),
			c->miss[D4PREFETCH+D4XMISC] / NONZERO(c->fetch[D4PREFETCH+D4XMISC]));

		if (c->flags & D4F_CCC) {
			demand_comp_data = c->comp_miss[D4PREFETCH+D4XMISC] 
			  + c->comp_miss[D4PREFETCH+D4XREAD]
		      	  + c->comp_miss[D4PREFETCH+D4XWRITE];
			demand_comp_alltype = demand_comp_data 
			  + c->comp_miss[D4PREFETCH+D4XINSTRN];
			demand_cap_data = c->cap_miss[D4PREFETCH+D4XMISC] 
			  + c->cap_miss[D4PREFETCH+D4XREAD]
		      	  + c->cap_miss[D4PREFETCH+D4XWRITE];
			demand_cap_alltype = demand_cap_data 
			  + c->cap_miss[D4PREFETCH+D4XINSTRN];
			demand_conf_data = c->conf_miss[D4PREFETCH+D4XMISC] 
			  + c->conf_miss[D4PREFETCH+D4XREAD]
		      	  + c->conf_miss[D4PREFETCH+D4XWRITE];
			demand_conf_alltype = demand_conf_data 
			  + c->conf_miss[D4PREFETCH+D4XINSTRN];

			printf(	"   PF compulsory misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_comp_alltype,
				c->comp_miss[D4PREFETCH+D4XINSTRN],
				demand_comp_data,
				c->comp_miss[D4PREFETCH+D4XREAD],
				c->comp_miss[D4PREFETCH+D4XWRITE],
				c->comp_miss[D4PREFETCH+D4XMISC]);

			printf(	"   PF capacity misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_cap_alltype,
				c->cap_miss[D4PREFETCH+D4XINSTRN],
				demand_cap_data,
				c->cap_miss[D4PREFETCH+D4XREAD],
				c->cap_miss[D4PREFETCH+D4XWRITE],
				c->cap_miss[D4PREFETCH+D4XMISC]);

			printf(	"   PF conflict misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_conf_alltype,
				c->conf_miss[D4PREFETCH+D4XINSTRN],
				demand_conf_data,
				c->conf_miss[D4PREFETCH+D4XREAD],
				c->conf_miss[D4PREFETCH+D4XWRITE],
				c->conf_miss[D4PREFETCH+D4XMISC]);

			printf(	"   PF compulsory fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				demand_comp_alltype / NONZERO(prefetch_alltype),
				c->comp_miss[D4PREFETCH+D4XINSTRN] / NONZERO(c->miss[D4PREFETCH+D4XINSTRN]),
				demand_comp_data / NONZERO(prefetch_data),
				c->comp_miss[D4PREFETCH+D4XREAD] / NONZERO(c->miss[D4PREFETCH+D4XREAD]),
				c->comp_miss[D4PREFETCH+D4XWRITE] / NONZERO(c->miss[D4PREFETCH+D4XWRITE]),
				c->comp_miss[D4PREFETCH+D4XMISC] / NONZERO(c->miss[D4PREFETCH+D4XMISC]));

			printf(	"   PF capacity fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				demand_cap_alltype / NONZERO(prefetch_alltype),
				c->cap_miss[D4PREFETCH+D4XINSTRN] / NONZERO(c->miss[D4PREFETCH+D4XINSTRN]),
				demand_cap_data / NONZERO(prefetch_data),
				c->cap_miss[D4PREFETCH+D4XREAD] / NONZERO(c->miss[D4PREFETCH+D4XREAD]),
				c->cap_miss[D4PREFETCH+D4XWRITE] / NONZERO(c->miss[D4PREFETCH+D4XWRITE]),
				c->cap_miss[D4PREFETCH+D4XMISC] / NONZERO(c->miss[D4PREFETCH+D4XMISC]));

			printf(	"   PF conflict fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				demand_conf_alltype / NONZERO(prefetch_alltype),
				c->conf_miss[D4PREFETCH+D4XINSTRN] / NONZERO(c->miss[D4PREFETCH+D4XINSTRN]),
				demand_conf_data / NONZERO(prefetch_data),
				c->conf_miss[D4PREFETCH+D4XREAD] / NONZERO(c->miss[D4PREFETCH+D4XREAD]),
				c->conf_miss[D4PREFETCH+D4XWRITE] / NONZERO(c->miss[D4PREFETCH+D4XWRITE]),
				c->conf_miss[D4PREFETCH+D4XMISC] / NONZERO(c->miss[D4PREFETCH+D4XMISC]));
		}

		printf(	" Total Misses		%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			demand_alltype + prefetch_alltype,
			c->miss[D4XINSTRN] + c->miss[D4PREFETCH+D4XINSTRN],
			demand_data + prefetch_data,
			c->miss[D4XREAD] + c->miss[D4PREFETCH+D4XREAD],
			c->miss[D4XWRITE] + c->miss[D4PREFETCH+D4XWRITE],
			c->miss[D4XMISC] + c->miss[D4PREFETCH+D4XMISC]);

		printf(	"  Total miss rate	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			(demand_alltype + prefetch_alltype) / NONZERO(demand_fetch_alltype + prefetch_fetch_alltype),
			(c->miss[D4XINSTRN] + c->miss[D4PREFETCH+D4XINSTRN]) / NONZERO(c->fetch[D4XINSTRN] + c->fetch[D4PREFETCH+D4XINSTRN]),
			(demand_data + prefetch_data) / NONZERO(demand_fetch_data + prefetch_fetch_data),
			(c->miss[D4XREAD] + c->miss[D4PREFETCH+D4XREAD]) / NONZERO(c->fetch[D4XREAD] + c->fetch[D4PREFETCH+D4XREAD]),
			(c->miss[D4XWRITE] + c->miss[D4PREFETCH+D4XWRITE]) / NONZERO(c->fetch[D4XWRITE] + c->fetch[D4PREFETCH+D4XWRITE]),
			(c->miss[D4XMISC] + c->miss[D4PREFETCH+D4XMISC]) / NONZERO(c->fetch[D4XMISC] + c->fetch[D4PREFETCH+D4XMISC]));

	} /* End of prefetch misses. */
	printf("\n");

	/*
	 * End of Misses Numbers
	 */


	/*
	 * Print Block Miss Numbers
	 */
	if (c->lg2subblocksize != c->lg2blocksize) {
		demand_data = c->blockmiss[D4XMISC] 
			  + c->blockmiss[D4XREAD]
		      	  + c->blockmiss[D4XWRITE];
		demand_alltype = demand_data 
			  + c->blockmiss[D4XINSTRN];

		printf(	" Demand Block Misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
			demand_alltype,
			c->blockmiss[D4XINSTRN],
			demand_data,
			c->blockmiss[D4XREAD],
			c->blockmiss[D4XWRITE],
			c->blockmiss[D4XMISC]);

		printf(	"  DB miss rate		%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
			demand_alltype / NONZERO(demand_fetch_alltype),
			c->blockmiss[D4XINSTRN] / NONZERO(c->fetch[D4XINSTRN]),
			demand_data / NONZERO(demand_fetch_data),
			c->blockmiss[D4XREAD] / NONZERO(c->fetch[D4XREAD]),
			c->blockmiss[D4XWRITE] / NONZERO(c->fetch[D4XWRITE]),
			c->blockmiss[D4XMISC] / NONZERO(c->fetch[D4XMISC]));

		if (c->flags & D4F_CCC) {
			demand_comp_data = c->comp_blockmiss[D4XMISC] 
			  + c->comp_blockmiss[D4XREAD]
		      	  + c->comp_blockmiss[D4XWRITE];
			demand_comp_alltype = demand_comp_data 
			  + c->comp_blockmiss[D4XINSTRN];
			demand_cap_data = c->cap_blockmiss[D4XMISC] 
			  + c->cap_blockmiss[D4XREAD]
		      	  + c->cap_blockmiss[D4XWRITE];
			demand_cap_alltype = demand_cap_data 
			  + c->cap_blockmiss[D4XINSTRN];
			demand_conf_data = c->conf_blockmiss[D4XMISC] 
			  + c->conf_blockmiss[D4XREAD]
		      	  + c->conf_blockmiss[D4XWRITE];
			demand_conf_alltype = demand_conf_data 
			  + c->conf_blockmiss[D4XINSTRN];

			printf(	"   DB compulsory misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_comp_alltype,
				c->comp_blockmiss[D4XINSTRN],
				demand_comp_data,
				c->comp_blockmiss[D4XREAD],
				c->comp_blockmiss[D4XWRITE],
				c->comp_blockmiss[D4XMISC]);

			printf(	"   DB capacity misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_cap_alltype,
				c->cap_blockmiss[D4XINSTRN],
				demand_cap_data,
				c->cap_blockmiss[D4XREAD],
				c->cap_blockmiss[D4XWRITE],
				c->cap_blockmiss[D4XMISC]);

			printf(	"   DB conflict misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_conf_alltype,
				c->conf_blockmiss[D4XINSTRN],
				demand_conf_data,
				c->conf_blockmiss[D4XREAD],
				c->conf_blockmiss[D4XWRITE],
				c->conf_blockmiss[D4XMISC]);

			printf(	"   DB compulsory fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				demand_comp_alltype / NONZERO(demand_alltype),
				c->comp_blockmiss[D4XINSTRN] / NONZERO(c->blockmiss[D4XINSTRN]),
				demand_comp_data / NONZERO(demand_data),
				c->comp_blockmiss[D4XREAD] / NONZERO(c->blockmiss[D4XREAD]),
				c->comp_blockmiss[D4XWRITE] / NONZERO(c->blockmiss[D4XWRITE]),
				c->comp_blockmiss[D4XMISC]) / NONZERO(c->blockmiss[D4XMISC]);

			printf(	"   DB capacity fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				demand_cap_alltype / NONZERO(demand_alltype),
				c->cap_blockmiss[D4XINSTRN] / NONZERO(c->blockmiss[D4XINSTRN]),
				demand_cap_data / NONZERO(demand_data),
				c->cap_blockmiss[D4XREAD] / NONZERO(c->blockmiss[D4XREAD]),
				c->cap_blockmiss[D4XWRITE] / NONZERO(c->blockmiss[D4XWRITE]),
				c->cap_blockmiss[D4XMISC] / NONZERO(c->blockmiss[D4XMISC]));
	    
			printf(	"   DB conflict fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				demand_conf_alltype / NONZERO(demand_alltype),
				c->conf_blockmiss[D4XINSTRN] / NONZERO(c->blockmiss[D4XINSTRN]),
				demand_conf_data / NONZERO(demand_data),
				c->conf_blockmiss[D4XREAD] / NONZERO(c->blockmiss[D4XREAD]),
				c->conf_blockmiss[D4XWRITE] / NONZERO(c->blockmiss[D4XWRITE]),
				c->conf_blockmiss[D4XMISC] / NONZERO(c->blockmiss[D4XMISC]));
		}

		/*
		 * Prefetch block misses?
		 */
		if (c->prefetchf != d4prefetch_none) {
			prefetch_data = c->blockmiss[D4PREFETCH+D4XMISC] 
				  + c->blockmiss[D4PREFETCH+D4XREAD]
		      		  + c->blockmiss[D4PREFETCH+D4XWRITE];
			prefetch_alltype = prefetch_data 
				  + c->blockmiss[D4PREFETCH+D4XINSTRN];

			printf(	" Prefetch Block Misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				prefetch_alltype,
				c->blockmiss[D4PREFETCH+D4XINSTRN],
				prefetch_data,
				c->blockmiss[D4PREFETCH+D4XREAD],
				c->blockmiss[D4PREFETCH+D4XWRITE],
				c->blockmiss[D4PREFETCH+D4XMISC]);

			printf(	"  PFB miss rate		%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				prefetch_alltype / NONZERO(prefetch_fetch_alltype),
				c->blockmiss[D4PREFETCH+D4XINSTRN] / NONZERO(c->fetch[D4PREFETCH+D4XINSTRN]),
				prefetch_data / NONZERO(prefetch_fetch_data),
				c->blockmiss[D4PREFETCH+D4XREAD] / NONZERO(c->fetch[D4PREFETCH+D4XREAD]),
				c->blockmiss[D4PREFETCH+D4XWRITE] / NONZERO(c->fetch[D4PREFETCH+D4XWRITE]),
				c->blockmiss[D4PREFETCH+D4XMISC] / NONZERO(c->fetch[D4PREFETCH+D4XMISC]));

			if (c->flags & D4F_CCC) {
				demand_comp_data = c->comp_blockmiss[D4PREFETCH+D4XMISC] 
					+ c->comp_blockmiss[D4PREFETCH+D4XREAD]
					+ c->comp_blockmiss[D4PREFETCH+D4XWRITE];
				demand_comp_alltype = demand_comp_data 
					+ c->comp_blockmiss[D4PREFETCH+D4XINSTRN];
				demand_cap_data = c->cap_blockmiss[D4PREFETCH+D4XMISC] 
					+ c->cap_blockmiss[D4PREFETCH+D4XREAD]
					+ c->cap_blockmiss[D4PREFETCH+D4XWRITE];
				demand_cap_alltype = demand_cap_data 
					+ c->cap_blockmiss[D4PREFETCH+D4XINSTRN];
				demand_conf_data = c->conf_blockmiss[D4PREFETCH+D4XMISC] 
					+ c->conf_blockmiss[D4PREFETCH+D4XREAD]
					+ c->conf_blockmiss[D4PREFETCH+D4XWRITE];
				demand_conf_alltype = demand_conf_data 
					+ c->conf_blockmiss[D4PREFETCH+D4XINSTRN];

				printf(	"   PFB comp misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
					demand_comp_alltype,
					c->comp_blockmiss[D4PREFETCH+D4XINSTRN],
					demand_comp_data,
					c->comp_blockmiss[D4PREFETCH+D4XREAD],
					c->comp_blockmiss[D4PREFETCH+D4XWRITE],
					c->comp_blockmiss[D4PREFETCH+D4XMISC]);

				printf(	"   PFB cap misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
					demand_cap_alltype,
					c->cap_blockmiss[D4PREFETCH+D4XINSTRN],
					demand_cap_data,
					c->cap_blockmiss[D4PREFETCH+D4XREAD],
					c->cap_blockmiss[D4PREFETCH+D4XWRITE],
					c->cap_blockmiss[D4PREFETCH+D4XMISC]);

				printf(	"   PFB conf misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
					demand_conf_alltype,
					c->conf_blockmiss[D4PREFETCH+D4XINSTRN],
					demand_conf_data,
					c->conf_blockmiss[D4PREFETCH+D4XREAD],
					c->conf_blockmiss[D4PREFETCH+D4XWRITE],
					c->conf_blockmiss[D4PREFETCH+D4XMISC]);

				printf(	"   PFB comp fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
					demand_comp_alltype / NONZERO(prefetch_alltype),
					c->comp_blockmiss[D4PREFETCH+D4XINSTRN] / NONZERO(c->blockmiss[D4PREFETCH+D4XINSTRN]),
					demand_comp_data / NONZERO(prefetch_data),
					c->comp_blockmiss[D4PREFETCH+D4XREAD] / NONZERO(c->blockmiss[D4PREFETCH+D4XREAD]),
					c->comp_blockmiss[D4PREFETCH+D4XWRITE] / NONZERO(c->blockmiss[D4PREFETCH+D4XWRITE]),
					c->comp_blockmiss[D4PREFETCH+D4XMISC] / NONZERO(c->blockmiss[D4PREFETCH+D4XMISC]));
	    
				printf(	"   PFB cap fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
					demand_cap_alltype / NONZERO(prefetch_alltype),
					c->cap_blockmiss[D4PREFETCH+D4XINSTRN] / NONZERO(c->blockmiss[D4PREFETCH+D4XINSTRN]),
					demand_cap_data / NONZERO(prefetch_data),
					c->cap_blockmiss[D4PREFETCH+D4XREAD] / NONZERO(c->blockmiss[D4PREFETCH+D4XREAD]),
					c->cap_blockmiss[D4PREFETCH+D4XWRITE] / NONZERO(c->blockmiss[D4PREFETCH+D4XWRITE]),
					c->cap_blockmiss[D4PREFETCH+D4XMISC] / NONZERO(c->blockmiss[D4PREFETCH+D4XMISC]));

	    			printf(	"   PFB conf fract	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
					demand_conf_alltype / NONZERO(prefetch_alltype),
					c->conf_blockmiss[D4PREFETCH+D4XINSTRN] / NONZERO(c->blockmiss[D4PREFETCH+D4XINSTRN]),
					demand_conf_data / NONZERO(prefetch_data),
					c->conf_blockmiss[D4PREFETCH+D4XREAD] / NONZERO(c->blockmiss[D4PREFETCH+D4XREAD]),
					c->conf_blockmiss[D4PREFETCH+D4XWRITE] / NONZERO(c->blockmiss[D4PREFETCH+D4XWRITE]),
					c->conf_blockmiss[D4PREFETCH+D4XMISC] / NONZERO(c->blockmiss[D4PREFETCH+D4XMISC]));
			}

			printf(	" Total Block Misses	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f	%12.0f\n",
				demand_alltype + prefetch_alltype,
				c->blockmiss[D4XINSTRN] + c->blockmiss[D4PREFETCH+D4XINSTRN],
				demand_data + prefetch_data,
				c->blockmiss[D4XREAD] + c->blockmiss[D4PREFETCH+D4XREAD],
				c->blockmiss[D4XWRITE] + c->blockmiss[D4PREFETCH+D4XWRITE],
				c->blockmiss[D4XMISC] + c->blockmiss[D4PREFETCH+D4XMISC]);

			printf(	"  Tot blk miss rate	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f	%12.4f\n",
				(demand_alltype + prefetch_alltype) / NONZERO(demand_fetch_alltype + prefetch_fetch_alltype),
				(c->blockmiss[D4XINSTRN] + c->blockmiss[D4PREFETCH+D4XINSTRN]) / NONZERO(c->fetch[D4XINSTRN] + c->fetch[D4PREFETCH+D4XINSTRN]),
				(demand_data + prefetch_data) / NONZERO(demand_fetch_data + prefetch_fetch_data),
				(c->blockmiss[D4XREAD] + c->blockmiss[D4PREFETCH+D4XREAD]) / NONZERO(c->fetch[D4XREAD] + c->fetch[D4PREFETCH+D4XREAD]),
				(c->blockmiss[D4XWRITE] + c->blockmiss[D4PREFETCH+D4XWRITE]) / NONZERO(c->fetch[D4XWRITE] + c->fetch[D4PREFETCH+D4XWRITE]),
				(c->blockmiss[D4XMISC] + c->blockmiss[D4PREFETCH+D4XMISC]) / NONZERO(c->fetch[D4XMISC] + c->fetch[D4PREFETCH+D4XMISC]));
		} /* End of prefetch block misses */
		printf("\n");
	} /* End of block misses */
	/*
	 * End of Block Misses Numbers
	 */

	/*
	 * Report multiblock and traffic to/from memory
	 */
	printf( " Multi-block refs      %12.0f\n",
		c->multiblock);
	printf(	" Bytes From Memory	%12.0f\n",
		c->bytes_read);
	printf(	" ( / Demand Fetches)	%12.4f\n",
		c->bytes_read / NONZERO(demand_fetch_alltype));
	printf(	" Bytes To Memory	%12.0f\n",
		c->bytes_written);
	printf(	" ( / Demand Writes)	%12.4f\n",
		c->bytes_written / NONZERO(c->fetch[D4XWRITE]));
	printf(	" Total Bytes r/w Mem	%12.0f\n",
		c->bytes_read + c->bytes_written);
	printf(	" ( / Demand Fetches)	%12.4f\n",
		(c->bytes_read + c->bytes_written) / NONZERO(demand_fetch_alltype));
	printf("\n");
}

#undef NONZERO


/*
 * Called to produce each address trace record
 */
d4memref
next_trace_item(G *g)
{
	d4memref r;
	static int once = 1;
	static int discard = 0;
	static int hastoggled = 0;
	printf("%s %d\n", __FUNCTION__, __LINE__);
	if (once) {
		once = 0;
		if (g->on_trigger != 0)
			discard = 1;	/* initially discard until trigger address seen */
		if (g->skipcount > 0) {
			double tskipcount = g->skipcount;
			do {
				r = input_function();
				if (r.accesstype == D4TRACE_END) {
					fprintf (stderr, "%s warning: input ended "
						 "before -skipcount satisfied\n", g->progname);
					return r;
				}
			} while ((tskipcount -= 1) > 0);
		}
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
	while (1) {
		r = input_function();
		if (r.accesstype == D4TRACE_END) {
			if ((g->on_trigger != 0 || g->off_trigger != 0) && !hastoggled)
				fprintf (stderr, "%s warning: trace discard "
					 "trigger addresses were not matched\n", g->progname);
			else if (discard == 0 && g->off_trigger != 0)
				fprintf (stderr, "%s warning: tail end of trace not discarded\n", g->progname);
			return r;
		}
		if (r.address != 0) {	/* valid triggers must be != 0 */
			if ((discard != 0 && g->on_trigger == r.address) ||
			    (discard == 0 && g->off_trigger == r.address)) {
				discard ^= 1;	/* toggle */
				hastoggled = 1;
				continue;	/* discard the trigger itself */
			}
		}
		if (!discard)
			return r;
	}
}


/*
 * Called to initialize all caches based on args
 * Die with an error message if there are serious problems.
 */
void
initialize_caches (G *g, d4cache **icachep, d4cache **dcachep)
{
	static char memname[] = "memory";
	int i, lev, idu;
	d4cache	*c = NULL,	/* avoid `may be used uninitialized' warning in gcc */
		*ci,
		*cd;
	printf("%s %d\n", __FUNCTION__, __LINE__);
	g->mem = cd = ci = d4new(g, NULL);
	if (ci == NULL)
		die (g, "cannot create simulated memory\n");
	ci->name = memname;
	printf("%s %d\n", __FUNCTION__, __LINE__);
	for (lev = g->maxlevel-1;  lev >= 0;  lev--) {
		for (idu = 0;  idu < 3;  idu++) {
			if (g->level_size[idu][lev] != 0) {
				switch (idu) {
				case 0:	cd = ci = c = d4new (g, ci); break;	/* u */
				case 1:	     ci = c = d4new (g, ci); break;	/* i */
				case 2:	     cd = c = d4new (g, cd); break;	/* d */
				}
				if (c == NULL)
					die (g, "cannot create level %d %ccache\n",
					     lev+1, idu==0?'u':(idu==1?'i':'d'));
				init_1cache (g, c, lev, idu);
				g->levcache[idu][lev] = c;
			}
		}
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
	i = d4setup(g);
	printf("%s %d\n", __FUNCTION__, __LINE__);
	if (i != 0)
		die (g, "cannot complete cache initializations; d4setup = %d\n", i);
	*icachep = ci;
	*dcachep = cd;
}


#if !D4CUSTOM	/* a customized version cannot be further customized */
/*
 * Create a customized version of dineroIV.  This is called after everything
 * is all set up and we would otherwise be ready to begin simulation.
 *
 * We create a temporary .c file, containing initializers for the
 * command line argument arrays (level_*, etc.), handled mostly by
 * args[].customf.  We also call d4customize to add customized
 * d4ref code for all of our caches in the same file.
 * We rebuild dineroIV by running $D4_SRC/make with extra arguments,
 * and finally exec the new program.
 */
void
customize_caches(G *g)
{
	FILE *f = NULL;
	char fname[100];
	int pid = getpid();
	struct arglist *adesc;
	struct stat st;
	char *cmdline = malloc(4096);	/* XXX add overflow checking or make this more dynamic */
	char *psrc, *plib;
	int x;

	if (cmdline == NULL)
		die (g, "no memory for custom make command line\n");
	sprintf (fname, "/tmp/d4custom%d.c", pid);
	f = fopen (fname, "w");
	if (f == NULL)
		die (g, "can't create file %s for writing (%s)\n", fname, strerror(errno));
	d4customize(g, f);

	/* call all customf functions */
	fprintf (f, "\n#include \"cmdargs.h\"\n");
	for (adesc = g->args;  adesc->optstring != NULL;  adesc++) {
		if (adesc->customf != NULL)
			adesc->customf (adesc, f);
	}
	fprintf (f, "int maxlevel = %d;\n", g->maxlevel);

	fclose (f);

	/* run make */
	psrc = getenv ("D4_SRC");
	if (psrc == NULL || *psrc == 0)
		psrc = ".";
	plib = getenv ("D4_LIB");
	if (plib == NULL || *plib == 0) {
		plib = malloc (strlen(psrc)+strlen("/libd4.a")+1);
		if (plib == NULL)
			die (g, "no memory for libd4.a pathname\n");
		strcpy (plib, psrc);
		strcat (plib, "/libd4.a");
	}

	/* try to catch common errors */
	sprintf (cmdline, "%s/Makefile", psrc);
	if (stat (cmdline, &st) < 0)
		die (g, "There is no %s%s\n", cmdline,
		     getenv("D4_SRC")==NULL ? "; try setting D4_SRC" : "");
	sprintf (cmdline, "%s/d4.h", psrc);
	if (stat (cmdline, &st) < 0)
		die (g, "There is no %s%s\n", cmdline,
		     getenv("D4_SRC")==NULL ? "; try setting D4_SRC" : "");
	if (stat (plib, &st) < 0)
		die (g, "There is no %s%s\n", plib,
		     getenv("D4_LIB")==NULL ? "; try setting D4_LIB" : "");

	sprintf (cmdline, "make -s -f %s/Makefile %s CUSTOM_NAME=%s "
		 "CUSTOM_C=%s D4_SRC=%s D4_LIB=%s\n",
		 psrc, g->customname, g->customname, fname, psrc, plib);
	x = system (cmdline);

#if 1	/* remove custom source file */
	(void)unlink(fname);
#endif
	if (x != 0)
		die (g, "can't make %s: %s returned %d\n", g->customname, cmdline, x);

	/* exec g->customname using cust_argc, cust_argv */
	g->cust_argv[0] = g->customname;
	g->cust_argv[g->cust_argc++] = NULL;
	x = execv (g->customname, (char**)(g->cust_argv)); /* cast avoids warnings */
	die (g, "cannot exec custom version %s: %s\n", g->customname, strerror(x));
}
#endif	/* !D4CUSTOM */


/*
 * Complain and terminate
 */
void
die (G *g, const char *fmt, ...)
{
	va_list ap;
	fflush (stdout);
	fprintf (stderr, "%s: ", g->progname);
	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);
	exit (1);
}


/*
 * Produce short help message for improper usage, then terminate
 */
void
shorthelp (G *g, const char *fmt, ...)
{
	va_list ap;
	fflush (stdout);
	fprintf (stderr, "%s: ", g->progname);
	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);
	fprintf (stderr, "Consult Dinero IV documentation\n"
		 "or run \"%s -help\" for usage information.\n", g->progname);
	exit (1);
}


/*
 * Ceiling of log base 2
 * Return -1 for clog2(0)
 */
int
clog2 (unsigned int x)
{
	int i;

	for (i = -1;  x != 0;  i++)
		x >>= 1;
	return i;
}

#define MAXCORE 16
G *gg[MAXCORE];

int do_cache_ref(int core_id, d4memref r)
{
	int miss_cnt = 0;

	if (core_id < 0 || core_id >= MAXCORE) {
		printf("ERROR: invalid core id\n");
		return -1;
	}
	G *g = gg[core_id];

	//r = next_trace_item(g);
	if (r.accesstype == D4TRACE_END)
		goto done;

	if (g->maxcount != 0 && g->tmaxcount >= g->maxcount) {
		printf ("---Maximum address count exceeded.\n");
		return -1;
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
	d4cache *ci = g->ci;
	d4cache *cd = g->cd;
	printf("%s %d\n", __FUNCTION__, __LINE__);
	switch (r.accesstype) {
	case D4XINSTRN:	  miss_cnt = d4ref (g, ci, r);  printf("miss %d\n", miss_cnt); break;
	case D4XINVAL:	  miss_cnt = d4ref (g, ci, r);  printf("miss %d\n", miss_cnt); /* fall through */ 
	default:	  miss_cnt = d4ref (g, cd, r);  printf("miss %d\n", miss_cnt); break;
	}
	g->tmaxcount += 1;
	if (g->tintcount > 0 && (g->tintcount -= 1) <= 0) {
		dostats(g);
		g->tintcount = g->stat_interval;
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
	if (g->flcount > 0 && (g->flcount -= 1) <= 0) {
		/* flush cache = copy back and invalidate */
		r.accesstype = D4XCOPYB;
		r.address = 0;
		r.size = 0;
		miss_cnt = d4ref (g, cd, r); printf("miss %d\n", miss_cnt);
		r.accesstype = D4XINVAL;
		miss_cnt = d4ref (g, ci, r); printf("miss %d\n", miss_cnt);
		if (ci != cd) {
			miss_cnt = d4ref (g, cd, r); printf("miss %d\n", miss_cnt);
		}
		g->flcount = g->flushcount;
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
	return miss_cnt;
 done:
	return -1;
}

// int do_cache_ref_core(int core_id, 
// 		      d4memref r, 
// 		      d4cache *ci, d4cache *cd, 
// 		      double *tmaxcount, double *flcount, 
// 		      double tintcount)
// {

// 	do_cache_ref(g, r, g->ci, g->cd, g->tmaxcount, g->flcount, g->tintcount);
// }

int do_cache_init(int core_id)
{
	printf("%s %d\n", __FUNCTION__, __LINE__);
	if (core_id >= MAXCORE) {
		printf("ERROR: exceed core number limit\n");
		return -1;
	}
	printf("%s %d\n", __FUNCTION__, __LINE__);
	G *g = (G*)malloc(sizeof(G));	
	if (g == NULL)
		printf("g malloc failed\n");
	printf("%s %d\n", __FUNCTION__, __LINE__);
	g->progname = "dineroIV";
	g->cust_argc = 1;
	g->informat = DEFVAL_informat;
	g->nextcacheid = 1;

	doargs_simple(g);
	printf("%s %d\n", __FUNCTION__, __LINE__);
	verify_options(g);
	printf("%s %d\n", __FUNCTION__, __LINE__);
	initialize_caches (g, &g->ci, &g->cd);
	printf("%s %d\n", __FUNCTION__, __LINE__);

#if !D4CUSTOM
	if (g->customname != NULL) {
		customize_caches(g);
		/* never returns */
	}
#endif
	if (g->cd == NULL)
		g->cd = g->ci;	/* for unified L1 cache */

	gg[core_id] = g;
	printf("%s %d\n", __FUNCTION__, __LINE__);
	return core_id;
}

