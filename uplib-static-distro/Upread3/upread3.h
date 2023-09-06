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
#define UR_VERSION "V3.1 Feb 2002 KIRUG - L.Schomaker, L.Vuurpijl, G.Abbink  - NICI"
#endif

/* Each output function has as input OFunction_Info *oinfo, containing
   fields required by the different output functions. If you add your own,
	and if you require extra fields, just put them in this struct.
*/

#define NOUTPUT_FUNCTIONS   5
#define _OUTPUT_UNIPEN   0
#define _OUTPUT_FEATCHAR 1
#define _OUTPUT_IMAGE    2
#define _OUTPUT_NAMEDXY  3
#define _OUTPUT_RAWXY    4

typedef struct {
	char outfile[512];
	int output_function;
	int mreq_nsamples; /* number of points to resample to -1 = don't resample       */
	int do_add_sincos; /* indicates if running angles (dx,dy) must be added         */
	int do_add_phi;    /* indicates if angle velocities (dphix,dphiy) must be added */
	double dfact;      /* factor for adjusting resampling (between [0,1]) */
	int width;         /* width of image */
	int height;        /* height of image */
	int brush;         /* for line generator */
	int margin;        /* for output_image */
	int im_frmt;       /* either O_XBM O_PPM or O_PGM */
	char canvas[512];  /* name of (pgm) image file with background */
	int clearcanvas;    /* clear canvas image after reading */
	int writer_code;    /* corresponding to file currently opened */
	char writer_codestr[1000]; /* derived from filename currently opened */
	int use_Z;
	double range;      /* -1. autoscale, otherwise tablet range */
	int norm_pos_size; /* 0=-RAWXY else c.o.g.=(0,0), sd(radius)=1 */
	int filledcontours; /* if closed contours (cocos): fill color */
	int nsmooth;        /* xy coordinate filtering (<=1 is no filtering) */
	int inkmodel;        /* inkmodel 0,1,2 etc. */
	int labeled_outfilnam; /* images: filename labeled with segname */
} OFunction_Info;

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
	, int named
	, Yfont *yfont);

#endif
