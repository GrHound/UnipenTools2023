
/**************************************************************************
*                                                                         *
*  UNIPEN PROJECT                                                         *
*                                                                         *
*    (c) Nijmegen Institute for Cognition and Information                 *
*                                                                         *
***************************************************************************
*                                                                         *
*  AUTHORS:                                                               *
*                                                                         *
*    Gerben H. Abbink, Lambert Schomaker and Louis Vuurpijl               *
*                                                                         *
*  DISCLAIMER:                                                            *
*                                                                         *
*    USER SHALL BE FREE TO USE AND COPY THIS SOFTWARE FREE OF CHARGE OR   *
*    FURTHER OBLIGATION.                                                  *
*                                                                         *
*    THIS SOFTWARE IS NOT OF PRODUCT QUALITY AND MAY HAVE ERRORS OR       *
*    DEFECTS.                                                             *
*                                                                         *
*    PROVIDER GIVES NO EXPRESS OR IMPLIED WARRANTY OF ANY KIND AND ANY    *
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR PURPOSE ARE    *
*    DISCLAIMED.                                                          *
*                                                                         *
*    PROVIDER SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL,      *
*    INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THIS   *
*    SOFTWARE.                                                            *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uplib.h>
#include <upsiglib.h>
#include <uphierarchy.h>

void hierDebugLevels (tUPLevel *levels, int n)
{
	int i;

	for (i=0;i<n;i++)
		fprintf (stderr,"level[%d] %d segments\n",i,levels[i].nsegments);
}


void AddChild (tUPHierarchy *c, tUPHierarchy *h)
{
	int n;
	
	if ((n=h->nchildren)==0) {
		up_alloc(h->children,tUPHierarchy *,n+1);
	}
	else {
		up_realloc(h->children,tUPHierarchy *,n+1);
	}
	h->children[n] = c;
	h->nchildren++;
}

void AdministrateHierarchy (tUPUnipen *p, tUPLevel *levels, tUPHierarchy *h)  /* is called for each .SEGMENT entry once */
{
	tUPHierarchy *c;                           /* h refers to the current segment under investigation */
	sigDelineation *h_del,*c_del;             /* c refers to any current segment belonging to any level c_lev */
	tUPHierarchy *segments;
	int h_lev=h->level;
	int i;

	h_del = h->del;
	if (h_del==NULL)
		return;

#ifdef _ALSO_ADMINISTRATE_PARENTS_
	/* first administrate a potential parent, in the upper hierarchy */

	if (h_lev!=0) {
		segments = levels[h_lev-1].segments;
		for (i=0;i<levels[h_lev-1].nsegments;i++) {
			c = &(segments[i]);
			if ((c_del=c->del)!=NULL) {
				if (sigSegmentInSegment(c_del,h_del)) { /* YES, c is a parent of h at level l */
					h->parent = c;
					break;
				}
			}
		}
	}
#endif
	/* and subsequently add administrate potential children in lower hierarchies */
	
	if (h_lev!=p->NrOfLevels-1) {
		segments = levels[h_lev+1].segments;
		for (i=0;i<levels[h_lev+1].nsegments;i++) {
			c = &(segments[i]);
			if ((c_del=c->del)!=NULL) {
				if (sigSegmentInSegment(h_del,c_del)) { /* YES, c is a child of h at level l */
					AddChild(c,h);
				}
			}
		}
	}
	h->administrated = 1;
}

int hierSearchParentIndex (tUPLevel *levels, tUPHierarchy *c)
{
	tUPHierarchy *p;
	int l = c->level;
	int i;

	for (i=0;i<levels[l].nsegments;i++) {
		p = &levels[l].segments[i];
		if (sigSegmentInSegment(p->del,c->del))
			return i;
	}
	return -1;
}

void hierAddHierarchy2Levels (tUPUnipen *p,tUPLevel *levels)
{
	int l,i;

	for (l=0;l<p->NrOfLevels;l++) {
		for (i=0;i<levels[l].nsegments;i++) {
			AdministrateHierarchy(p,levels,&(levels[l].segments[i]));
		}
	}
}

void hierCreateHierarchyEntry (tUPUnipen *pUnipen, tUPHierarchy *e, tUPEntry *entry, int level, int idx)
{
	e->entry = entry;
	e->name = upEntryName(entry);
	e->level = level;
	e->level_idx = idx;
	e->del = sigEntry2Delineation(pUnipen,entry);
}

void hierFillHierarchyEntry (
	tUPHierarchy *e, tUPEntry *entry,
	char *name, int level, int level_idx, sigDelineation *del)
{
	e->name            = name;
	e->level           = level;
	e->level_idx       = level_idx;
	e->administrated   = 0;
	e->entry           = entry;
	e->del             = del;
	e->entry_allocated = 1;
	e->nchildren = 0;
	e->children  = NULL;
	e->parent    = NULL;
}

void hierCopyHierarchyEntries (tUPHierarchy *dst, tUPHierarchy *src)
{
	hierFillHierarchyEntry (dst
		,upNewEntry(strdup(src->entry->Entry),0)
		,strdup(src->name)
		,src->level,src->level_idx
		,sigCopyDelineation(src->del)
	);	
}

void hierDeleteHierarchyEntry (tUPHierarchy *h)
{
	free(h->name);
	if (h->nchildren>0) {
		free (h->children);
	}
	if (h->del!=NULL) {
		sigFreeDelineation(h->del);
	}
	if (h->entry_allocated) {
		free(h->entry->Entry);
		free(h->entry);
	}
}

tUPLevel *hierCreateLevels (tUPUnipen *p)
{
	tUPLevel *levels,*level;
	int nfound,l;
	tUPEntry *entry;

	up_alloc (levels,tUPLevel,p->NrOfLevels);
	for (l=0;l<p->NrOfLevels;l++) {
		level = &(levels[l]);
		level->nsegments = p->NrOfSegmentsInLevel[l];
		if (level->nsegments==0) {
			fprintf (stderr,"level [%s] has no .SEGMENT entries!\n",p->Levels[l]);
			continue;
		}
		up_alloc(level->segments,tUPHierarchy,level->nsegments);
		nfound = 0;
		upResetSegments(p);
		while ((entry = upNextSegment(p,p->Levels[l],1))!=NULL) {
			hierCreateHierarchyEntry(p,&(level->segments[nfound]),entry,l,nfound);
			nfound++;
		}
		if (nfound!=level->nsegments) {
			fprintf (stderr,"expected %d %s SEGMENTS, found %d!\n",level->nsegments,p->Levels[l],nfound);
		}
		mCurrOffsetSegment(p) = 0;
		mCurrOffsetStream(p) = -1;
	}
	return levels;
}

void hierDeleteLevels (tUPUnipen *p,tUPLevel *levels)
{
	int i,l;

	for (l=0;l<p->NrOfLevels;l++) {
		for (i=0;i<levels[l].nsegments;i++)
			hierDeleteHierarchyEntry(&(levels[l].segments[i]));
		free(levels[l].segments);
	}
	free(levels);
}

tUPHierarchy *hierFindParent (tUPUnipen *p, tUPLevel *levels,int level, int idx)
{
	tUPHierarchy *h = &(levels[level].segments[idx]);

	if (!h->administrated) /* not marked yet, find them! */
		AdministrateHierarchy(p,levels,h);
	if (h->parent==NULL) {
		return NULL;
	}
	return h->parent;
}

static int child_sort (const void *p1, const void *p2)
{
	tUPHierarchy *c1 = *((tUPHierarchy **)p1);
	tUPHierarchy *c2 = *((tUPHierarchy **)p2);

	if (c1->del->delineations[0].first_streamnr==c2->del->delineations[0].first_streamnr)
		return (c1->del->delineations[0].first_samplenr-c2->del->delineations[0].first_samplenr);
	else
		return (c1->del->delineations[0].first_streamnr-c2->del->delineations[0].first_streamnr);
}

tUPHierarchy **hierFindChildren (tUPUnipen *p, tUPLevel *levels,int level, int idx)
{
	tUPHierarchy *h = &(levels[level].segments[idx]);

	if (!h->administrated) /* not marked yet, find them! */
		AdministrateHierarchy(p,levels,h);
	if (h->nchildren==0) {
		return NULL;
	}
	qsort((void *)h->children,h->nchildren,sizeof(tUPHierarchy *),child_sort);
	return h->children;
}
