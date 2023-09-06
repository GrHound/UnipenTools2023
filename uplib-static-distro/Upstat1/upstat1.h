#ifndef _UPREAD2_
#define _UPREAD2_

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

#ifndef SUN_SOL
#include <dirent.h>
#define DIR_DEFINE dirent
#else
#include <sys/dir.h>
#define DIR_DEFINE direct
#endif
 
#ifndef UR_VERSION
#define UR_VERSION "V1.0 Jun 2005 KIRUG - L.Schomaker, L.Vuurpijl, G.Abbink  - NICI"
#endif

#define _OUTPUT_UNIPEN   0
#define _OUTPUT_FEATCHAR 1
#define _OUTPUT_NAMEDXY  2
#define _OUTPUT_RAWXY    3
#define _OUTPUT_XPDF     4
#define _OUTPUT_YPDF     5
#define _OUTPUT_WPDF     6
#define _OUTPUT_HPDF     7
#define NOUTPUT_FUNCTIONS 8

#ifdef _MAIN
static char *output_function_names[] = {
        "unipen",
        "featchar",
        "namedxy",
        "rawxy",
        "xpdf",
        "ypdf",
        "wpdf",
        "hpdf",
        ""
};
#endif

/* Each output function has as input OFunction_Info *oinfo, containing
   fields required by the different output functions. If you add your own,
	and if you require extra fields, just put them in this struct.
*/



typedef struct {
	char outfile[512];
	int output_function;
	int m;             /* number of points to resample to -1 = don't resample       */
	int nbins;         /* number of PDF bins or number of features */
	int nsmo;          /* number of time to smooth with a 2-point boxcar */
	int do_add_sincos; /* indicates if running angles (dx,dy) must be added         */
	int do_add_phi;    /* indicates if angle velocities (dphix,dphiy) must be added */
	double dfact;      /* factor for adjusting resampling (between [0,1]) */
	int width;         /* width of image */
	int height;        /* height of image */
	int brush;         /* for line generator */
	int margin;        /* for output_image */
	int im_frmt;       /* either O_XBM O_PPM or O_PGM */
	int writer_code;   /* corresponding to file currently opened */
	char writer_codestr[1000]; /* derived from filename currently opened */
	char tag[1000];    /* output tag */
	int use_Z;
	double range;      /* -1. autoscale, otherwise tablet range */
	int norm_pos_size; /* 0=-RAWXY else c.o.g.=(0,0), sd(radius)=1 */
} OFunction_Info;

typedef struct {
   double xmin;
   double ymin;
   double xmax;
   double ymax;
   double width;
   double height;
   
   double ybase;
   double ycorpus;
} XY_Chunk;

extern void init_output_unipen (tUPUnipen *, FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo);
extern int output_unipen (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);

extern void init_output_featchar (FILE *fp_out, upsegQueryInfo *info
   , OFunction_Info *oinfo);
extern int output_featchar (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);


extern void init_output_featxy (FILE *fp_out, upsegQueryInfo *info
   , OFunction_Info *oinfo);
extern int output_featxy (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams
	, int named);


extern void init_output_xpdf (FILE *fp_out, upsegQueryInfo *info
   , OFunction_Info *oinfo);
extern int output_xpdf (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams
	, int named
	, XY_Chunk **xyobj
	, int *nobj
	);

#endif
