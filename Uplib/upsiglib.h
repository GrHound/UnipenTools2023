#ifndef _UPSIGLIB_
#define _UPSIGLIB_

#include "uplib.h"

#define INVALID_STREAMTYPE -1
#define PEN_UP   0
#define PEN_DOWN 1
#define DT       2

#define T_INSERTION  .15
#define AX_PEN_FORCE 100


typedef struct {
	int first_streamnr;
	int last_streamnr;
	int first_samplenr;
	int last_samplenr;
} sigSampleDelineation;

typedef struct {
/*	int level;
	int segm_nr;
*/	int ndels;
	sigSampleDelineation *delineations;
} sigDelineation;

typedef struct {
	int nstreams;
	int *nsamples;
	char **streams;
} sigCharStream;

/* char output */

extern int sigStreamType (char *);
extern void sigFreeDelineation (sigDelineation *);
extern sigDelineation *sigCopyDelineation (sigDelineation *);
extern sigDelineation *sigCreateBoundingDelineation (sigDelineation *);
extern void sigPrintDelineation (char *,sigDelineation *);
extern void sigOutputDelineation (FILE *fp, sigDelineation *del);

extern sigDelineation *sigParseDelineation (char *);
extern void adjust_delineation (tUPUnipen *, sigDelineation *);
extern sigDelineation *sigEntry2Delineation (tUPUnipen *, tUPEntry *);
extern sigCharStream *sigDelineation2Charstream (tUPUnipen *, sigDelineation *);
extern char *sigDelineation2String (sigDelineation *);
extern sigCharStream *sigGetSamples (tUPUnipen *, tUPEntry *);
extern sigCharStream *sigNextSegmentSamples (tUPUnipen *, char *);
extern void upSkipSegment (tUPUnipen *, tUPEntry *, char *);
extern void sigDeleteCharStream (sigCharStream *);
extern tUPEntry **sigSegmentsInSegment (tUPUnipen *, tUPEntry *, char *, int *);
extern int sigSegmentInSegment(sigDelineation *, sigDelineation *);
extern int sigNSamplesInDelineation (tUPUnipen *, sigDelineation *);

/* signal output, i.e. x,y,z coordinates */

#define RAW_SIGNAL      0
#define TIME_EQUI_DIST  1
#define SPACE_EQUI_DIST 2

typedef struct {
	int *x;
	int *y;
	int *z;
	int nsamples;
} sigSignal;

extern sigSignal *sigDelineation2Signal (tUPUnipen *, sigDelineation *);
extern sigSignal *sigCharstream2Signal (tUPUnipen *, tUPEntry *, int, sigCharStream *);
extern sigSignal *sigEntry2Signal (tUPUnipen *, tUPEntry *, int);
extern sigSignal *sigEntry2CleanSignal (tUPUnipen *, tUPEntry *, int, double);
extern int sigDelineation2CharSignal (tUPUnipen *, sigDelineation *
	,int *, int **, int **, int **);
extern int sigEntry2CharSignal (tUPUnipen *, tUPEntry *, int *, int **, int **, int **);
extern int sigCharStream2CharSignal (tUPUnipen *, sigCharStream *,int *, int **, int **, int **);

extern void sigPrintSignal (sigSignal *);
extern void sigDeleteSignal (sigSignal *);
extern void sigSegmentBoundsInSignal(tUPUnipen *, tUPEntry *,tUPEntry *,int *, int *);
extern void sigSegmentBoundsInDelineation (tUPUnipen *, sigDelineation  *
	, sigDelineation *,int *, int *);

extern sigSignal *sigPenstream2Signal(tUPUnipen *, int, int);

extern sigDelineation *sigBounds2Delineation (tUPUnipen *, tUPEntry *, sigSignal *,int,int);
extern sigDelineation *sigDelBounds2Delineation (tUPUnipen *, sigDelineation *, sigSignal *,int, int);

extern char *get_sample_from_string (tUPUnipen *pUnipen, char *ptr, int *x, int *y, int *z);

#endif
