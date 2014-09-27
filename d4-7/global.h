#ifndef _GLOBAL_H
#define _GLOBAL_H

/*
 * This structure describes a command line arg for Dinero IV.
 * Some args require a cache attribute prefix, -ln-idu, where
 * 1 <= n <= MAX_LEV and idu is "i", "d", or "", coresponding to an instruction,
 * data, or unified cache; this is all handled by choice of match function.
 *
 * The arglist structure also supports the help message,
 * summary info, and option customization.
 */ 

#ifndef MAX_LEV
#define MAX_LEV	5		/* allow -ln prefix no larger than this */
#endif

#ifndef MAX_ARGS
#define MAX_ARGS 128
#endif

struct arglist {
	const char *optstring;	  /* string to match, without -ln-idu if applicable */
	int pad;		  /* how many extra chars will help print? */
	void *var;		  /* scalar variable or array to modify */
	char *defstr;		  /* default value, as a string */
	const char *customstring; /* arg to use for custom version */
	const char *helpstring;	  /* string for help line */

				  /* function to recognize arg on command line */
	int (*match)(const char *opt, const struct arglist *);
				  /* valf is function to set value */
	void (*valf)(const char *opt, const char *arg, const struct arglist *);
				  /* customf produces definitions for custom version */
	void (*customf)(const struct arglist *, FILE *);
				  /* sumf prints summary line */
	void (*sumf)(const struct arglist *, FILE *);
				  /* help prints line for -help */
	void (*help)(const struct arglist *);
};


typedef struct _G {

/* some global variables */
char *progname;		       /* for error messages */
int optstringmax;			/* for help_* functions */
d4cache *levcache[3][MAX_LEV];		/* to locate cache by level and type */
d4cache *mem;				/* which cache represents simulated memory? */
#if !D4CUSTOM
const char **cust_argv;			/* for args to pass to custom version */
int cust_argc;			/* how many args for custom version */
char *customname;			/* for -custom, name of executable */
#endif

d4cache *ci, *cd;		//the i-cache and d-cache

/* Some globals, defined in cmdargs.c */
struct arglist args[MAX_ARGS];	/* defined in cmdargs.c */
int nargs;		/* num entries in args[] */
// int optstringmax;	/* longest option string */

//char *customname;	/* for -custom, name of executable */
//double skipcount;	/* for -skipcount */
//double flushcount;	/* for -flushcount */
//double maxcount;		/* for -maxcount */
//double stat_interval;	/* for -stat-interval */
//long on_trigger;		/* for -on-trigger */
//long off_trigger;	/* for -off-trigger */
//int stat_idcombine;	/* for -stat-idcombine */

/*
 * The size of each cache is given by
 *	level_size[idu][level]
 * where idu=0 for ucache, 1 for icache, 2 for dcache,
 * and 0=closest to processor, MAX_LEV-1 = closest to memory)
 */
//unsigned int level_size[3][MAX_LEV];

#if D4CUSTOM
#define D4_EXT extern
#else
#define D4_EXT
#endif
D4_EXT int maxlevel;	/* the highest level actually used */
D4_EXT unsigned int level_blocksize[3][MAX_LEV];
D4_EXT unsigned int level_subblocksize[3][MAX_LEV];
D4_EXT unsigned int level_size[3][MAX_LEV];
D4_EXT unsigned int level_assoc[3][MAX_LEV];
D4_EXT int level_doccc[3][MAX_LEV];
D4_EXT int level_replacement[3][MAX_LEV];
D4_EXT int level_fetch[3][MAX_LEV];
D4_EXT int level_walloc[3][MAX_LEV];
D4_EXT int level_wback[3][MAX_LEV];
D4_EXT int level_prefetch_abortpercent[3][MAX_LEV];
int level_prefetch_distance[3][MAX_LEV];

/*
 * command line defaults.
 * Make sure the DEFVAL and DEFSTR versions match
 * We don't really have to use fancy macro stuff for this do we?
 */
#define DEFVAL_assoc 1
#define  DEFSTR_assoc "1"
#define DEFVAL_repl 'l'
#define  DEFSTR_repl "l"
#define DEFVAL_fetch 'd'
#define  DEFSTR_fetch "d"
#define DEFVAL_walloc 'a'
#define  DEFSTR_walloc "a"
#define DEFVAL_wback 'a'
#define  DEFSTR_wback "a"
#define DEFVAL_informat 'D'
#define  DEFSTR_informat "D"

double skipcount;
double flushcount;
double maxcount;
double stat_interval;
int informat; // = DEFVAL_informat;
long on_trigger;
long off_trigger;
int stat_idcombine;

} G;

// extern G *g;

#endif	//#ifdef _GLOBAL_H
