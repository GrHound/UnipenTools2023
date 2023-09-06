#ifndef _UP_SEGMENT_IO_
#define _UP_SEGMENT_IO_

#include <uplib.h>
#include <upsiglib.h>

#define _QUERY_WIDTH            500.0
#define _QUERY_HEIGHT           500.0
#define _QUERY_LEVEL            "*"
#define _QUERY_NCOLUMNS         3
#define _QUERY_INITIAL_BOXSIZE  -1
#define _QUERY_BOXSIZE          200
#define _QUERY_XSCALE           1.0
#define _QUERY_YSCALE           1.0
#define _QUERY_PD               2.0
#define _QUERY_PU               0.5
#define _QUERY_FONTSIZE         15
#define QUERY_BLOCKSIZE         256
/* #define MAXQUERYSTRINGS         600 */
#define MAX_QUERY_STRING_LENGTH 256
#define MAXQUERYFILES           256
#define MAX_QUERY_FILE_LENGTH   128

typedef struct {
	tUPUnipen *pUnipen;
	double width;
	double height;
	double xscale;
	double yscale;
	double ystep;
	double boxsize;
	int ncolumns;
	int first;
	int last;
	int index;
	int current;
	int use_glob;
	char level[256];
	int nstrings;
	char **strings;
	int nfiles;
	char files[MAXQUERYFILES][MAX_QUERY_FILE_LENGTH];
	char *writing_style;
	int same_scale;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double margin;
	double minpres;
	double pd;
	double pu;
	int pointsize;
	int fontsize;
	int nolabel;
	int label_offset;
	int page_offsetx;
	int page_offsety;
	int label_offsetx;
	int label_offsety;
} upsegQueryInfo;

extern void upsegInitializeQueryInfo (upsegQueryInfo *);
extern sigSignal **upsegGetSignals (upsegQueryInfo *, int *, char ***);
extern tUPEntry **upsegGetEntries (upsegQueryInfo *, int *, char ***);
extern tUPEntry **upsegGetEntriesWithStreams (upsegQueryInfo *
	, int *, char ***, sigCharStream ***, char ***, char **);
extern void upsegAddQuery (upsegQueryInfo *, char *);
extern void upsegAddFile (upsegQueryInfo *, char *);
extern void upsegAddQueriesFromPatfile(upsegQueryInfo *, char *);

#endif
