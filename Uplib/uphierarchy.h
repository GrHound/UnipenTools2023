#ifndef __UPHIER_H__
#define __UPHIER_H__

#include <uplib.h>

typedef struct UPHIERARCHY {
	char *name;
	int level;
	int level_idx;
	int administrated;             /* tricky, no explanation                    */
	int nchildren;                 /* the segment's number of children          */
	struct UPHIERARCHY *parent;    /* the segment's parent or NULL              */
	struct UPHIERARCHY **children; /* the segment's children                    */
	tUPEntry *entry;               /* either allocated or pointing somewhere    */
	sigDelineation *del;           /* the --- possibly adjusted --- delineation */
	int entry_allocated;           /* 0 or 1                                    */
} tUPHierarchy;

typedef struct UPLEVEL {
	int nsegments;
	tUPHierarchy *segments;
} tUPLevel;

#define may_de_administrate(h) {\
	if (h->administrated) {\
		h->administrated = 0;\
		if (h->nchildren>0)\
			free(h->children);\
		h->nchildren = 0;\
	}\
}

extern void hierDebugLevels (tUPLevel *, int);

extern void hierAddHierarchy2Levels (tUPUnipen *,tUPLevel *);
extern tUPLevel *hierCreateLevels (tUPUnipen *);
extern void hierDeleteLevels (tUPUnipen *,tUPLevel *);
extern tUPHierarchy *hierFindParent (tUPUnipen *, tUPLevel *,int, int);
extern tUPHierarchy **hierFindChildren (tUPUnipen *, tUPLevel *,int, int);
extern void AddChild (tUPHierarchy *, tUPHierarchy *);
extern int hierSearchParentIndex  (tUPLevel *, tUPHierarchy *);

extern void hierCreateHierarchyEntry (tUPUnipen *, tUPHierarchy *, tUPEntry *, int, int);
extern void hierFillHierarchyEntry (tUPHierarchy *, tUPEntry *, char *, int, int, sigDelineation *);
extern void hierCopyHierarchyEntries (tUPHierarchy *, tUPHierarchy *);
extern void hierDeleteHierarchyEntry (tUPHierarchy *);

#endif
