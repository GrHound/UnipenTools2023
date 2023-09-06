#ifndef __UPLIB_H__
#define __UPLIB_H__

#ifndef UPLIB_VERSION_NR
#define UPLIB_VERSION_NR 3
#endif

#ifdef ultrix
#define strdup(s) strcpy(malloc(strlen(s)+1),s)
#endif


#define up_free(p) {if(p!=NULL) {free(p);p=NULL;}}
#define up_alloc(ptr,type,n) {\
	if (n<=0) {\
		fprintf (stderr,"FATAL: allocation with (%d) bytes!!!\n",n);\
		exit(1);\
	}\
	if ((ptr=(type *)calloc(n,sizeof(type)))==NULL) {\
		fprintf (stderr,"FATAL: allocation failure (%d bytes requested)!!! (exiting...)",n);\
		perror ("perror calloc: ");\
		exit(1);\
	}\
}
#define up_realloc(ptr,type,n) {\
	if (n<=0) {\
		fprintf (stderr,"FATAL: allocation with (%d) bytes!!!\n",n);\
		exit(1);\
	}\
	if ((ptr=(type *)realloc((void *)ptr,sizeof(type)*(n)))==NULL) {\
		fprintf (stderr,"FATAL: allocation failure (%d bytes requested)!!! (exiting...)",n);\
		perror ("perror realloc: ");\
		exit(1);\
	}\
}

/*
#include <dirent.h>
#include <varargs.h>
*/

#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cTrue  1
#define cFalse 0

/* match                                                                 */
typedef int upBool;


/* environment variable to search for unipen.def and include files */

#define cUnipenDefinition        "UNIPEN_DEFINITION_FILE"
#define cUnipenIncludePath       "UNIPEN_INCLUDE_PATH"
#define cIncludeInheritanceKey   ".INHERIT"

/*************************************************************************/
/*                                                                       */
/* error                                                                 */
/*                                                                       */
/*************************************************************************/

#define errOutOfMemory           "Out of memory"

#define errInvalidKeyword        "in [%s]: request for keyword \"%s\" unsucessful (not found)\n"
#define errMultipleCoords        "in [%s]: Multiple coordinates (%s)\n"
#define errMultipleHierarchies   "in [%s]: Multiple hierarchies (%s)\n"
#define errMultipleSets          "in [%s]: Multiple sets (%s)\n"
#define errNoCoord               "in [%s]: No coordinates (%s)\n"
#define errNoHierarchy           "in [%s]: No hierarchy (%s)\n"

/* .SEGMENT quality NOTE NOTE NOTE: some extra quality indications are
   added for the NICI .SEGMENT CHAR entries */

#define qUnknown  "?"
#define qBad      "BAD"
#define qOk       "OK"
#define qGood     "GOOD"
#define Q_UNKNOWN 0
#define Q_BAD     1
#define Q_OK      2
#define Q_GOOD    3

/* the NICI qualifications */

#define qLbl      "LBL"
#define qLbr      "LBR"
#define qLbg      "LBG"
#define qLbs      "LBS"
#define Q_LBL     Q_OK
#define Q_LBR     Q_BAD
#define Q_LBS     4

/* sub string                                                            */
#define cScheiders               "\t\n\r "
#define mIsScheider(mChar)       (strchr(cScheiders,mChar)!=NULL)
typedef struct _tUPSubString_
{
  char *String;
  int   Length;
  int   Offset;
} tUPSubString;

/* entry                                                                 */
typedef struct _tUPEntry_
{
  int   Count;
  char *Entry;
} tUPEntry;
typedef struct {
	tUPEntry *entry;
	int first_sample;
	int nsamples;
} tUPStream;

/* unipen                                                                */
#define cDefFile                 "unipen.def"
#define keyCoord                 ".COORD"
#define keyHierarchy             ".HIERARCHY"
#define keyInclude               ".INCLUDE"
#define keyKeyword               ".KEYWORD"
#define keyPenDown               ".PEN_DOWN"
#define keyPenLapse              ".DT"
#define keyPenUp                 ".PEN_UP"
#define keyReserve               ".RESERVE"
#define keySegment               ".SEGMENT"
#define keySampleRate            ".POINTS_PER_SECOND"
#define keySet                   ".START_SET"
#define cPenAdded                -1
#define mNrOfSets(p)             (p->NrOfEntries       [p->SetId])
#define mSampleRate(p)           (p->Entries[p->SampleRateId][0])
#define mNrOfHierarchies(p)      (p->NrOfEntries       [p->HierarchyId])
#define mNrOfCoords(p)           (p->NrOfEntries       [p->CoordId])
#define mNrOfSegments(p)         (p->NrOfEntries       [p->SegmentId])
#define mCurrOffsetSegment(p)    (p->CurrOffsetEntries [p->SegmentId])
#define mBoundedDelineation(p)   (p->bounded_delineation)
#define mUnknownDelineation(p)   (p->unknown_delineation)
#define mCurrOffsetStream(p)     (p->last_pen_stream)


/* some macros to administrate the ---  v e r y  --- tricky UNIPEN .SEGMENT ordering */

#define upRememberSegmentPosition(p) {\
	p->remembered_current_offset = mCurrOffsetSegment(p);\
	p->remembered_last_pen_stream = p->last_pen_stream;\
}

#define upResetSegmentPosition(p) {\
	mCurrOffsetSegment(p) = p->remembered_current_offset;\
	p->last_pen_stream    = p->remembered_last_pen_stream;\
}

#define upResetSegments(p) {\
	mCurrOffsetSegment(p) = -1;\
	p->last_pen_stream = -1;\
}

typedef struct _tUPUnipen_
{
	/* keywords */
	int    NrOfKeywords;
	char **Keywords;
	int NrOfMandatoryKeywords; /* added by Loe, Nov. '97 */
	char **MandatoryKeywords;

	/* short cuts */
	int SetId;
	int HierarchyId;
	int CoordId;
	int SegmentId;
	int IncludeId;
	int SampleRateId;
	int PenDownId;
	int PenUpId;
	int PenLapseId;

	/* delineation */
	int bounded_delineation;
	int unknown_delineation;

	/* allocated filecontents, for the Unipenfile and each included file */
	int    nunipens;
	char **file_contents;
	char *cur_file_open;
	char *cur_path;

	char **upIncludePaths;
	int NrIncludePaths;

	/* entries */
	int         TotalNrOfEntries;
	int        *NrOfEntries;
	int        *CurrOffsetEntries;
	int remembered_current_offset;
	tUPEntry ***Entries;
	tUPEntry **entry_sequence;
	
	/* streams */
	int remembered_last_pen_stream;
	int last_pen_stream;
	int NrOfPenStreams;
	int nvalid_streams;

	tUPStream *stream_sequence;
	int *valid_streams;

	/* hierarchy */
	int    NrOfLevels;
	int *NrOfSegmentsInLevel;
	char **Levels;
	
	/* coord                                                                  */
	/* from UNIPEN.DEF................                                        */
	/* X, Y, T, P, Z, B, RHO, THETA, PHI, including at least X and Y.         */
	int    NrOfCoords;
	char **Coords;
	int cur_level;
	int has_z;         /* -1=no, > -1=yes, indicating position */
	int has_p;         /* -1=no, > -1=yes, indicating position */
	int has_t;         /* -1=no, > -1=yes, indicating position */
	int has_b;         /* -1=no, > -1=yes, indicating position */
	int has_rho;       /* -1=no, > -1=yes, indicating position */
	int has_theta;     /* -1=no, > -1=yes, indicating position */
	int has_phi;       /* -1=no, > -1=yes, indicating position */

	/* verbosity: -1 is not much, >0 is tell me more */
	int verbosity;

} tUPUnipen;

#define logVerbosity(p) p->verbosity

/* memory and string                                                         */
extern void *upNewMemory (size_t);
extern char *upNewString (char *);
extern char *upNthString (char *, int);

/* entry                                                                     */
extern tUPEntry *upNewEntry (char *, int);
extern char *upEntryName (tUPEntry *);
extern char *upLevelName (tUPEntry *);
extern int upGetDoubleFromEntries(tUPUnipen *, char *, double *, int);
extern int upEntryQuality (tUPEntry *);
extern float upEntryFloatQuality (tUPEntry *);
extern int upAdministrateEntry (tUPUnipen *, char *, int *, int);

/* keywords */
#define upMANDATORY 1
#define upOPTIONAL  0
extern int upGetKeywordId (tUPUnipen *, char *);
extern void upRequireKeyword (tUPUnipen *, char *, int); /* the next two added Nov. '97 */
extern int upCheckMandatoryKeywords (tUPUnipen *, int);

/* unipen                                                                    */
extern char *upGetArgument (tUPUnipen *, char *, int);
extern tUPUnipen *upNewUnipen    (char *);
extern tUPUnipen *upDelUnipen    (tUPUnipen *);
extern upBool upReadNextFile (tUPUnipen *, char *, int *, int);
extern upBool     upNextFile     (tUPUnipen *,char *);
extern void upDoneFile (tUPUnipen *);

/* queries */
#define UP_ANY_LEVEL 9999999
extern int       upNSamples (char *);
extern int       upCurLevel (tUPUnipen *, char *);
extern tUPEntry  *upNextSegment  (tUPUnipen *, char *, int);
extern int       upSynchronizeSegmentOffsetAndCount (tUPUnipen *, tUPEntry *);
extern tUPEntry  *upSearchNextSegment  (tUPUnipen *, char *);
extern tUPEntry  *upSearchNextSegmentEntry  (tUPUnipen *, char *, tUPEntry *);
 
typedef struct {
	tUPEntry *entry;
	char *label;
} SegmentEntry;

extern SegmentEntry **construct_segment_entries (tUPUnipen *pUnipen
	, char *level, int *n, int dosort);

/* check dictionary */
typedef struct {
	int nwords;
	char **words;
} UnipenDictionary;

extern UnipenDictionary *upGetDictionary (tUPUnipen *);
extern int upCheckDictionary (tUPUnipen *, char *, char *);

extern int upGetVerbosity(tUPUnipen *pUnipen);
extern void upSetVerbosity(tUPUnipen *pUnipen, int log);

#endif
