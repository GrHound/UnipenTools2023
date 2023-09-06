#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

void upsegAddQueriesFromPatfile(upsegQueryInfo *info, char *fname)
{
	FILE *fp;
	char pattern[MAX_QUERY_STRING_LENGTH];

	if ((fp=fopen(fname,"r"))==NULL) {
		fprintf (stderr,"unable to open patfile '%s'!\n",fname);
		exit(1);
	}
	while (fgets(pattern,MAX_QUERY_STRING_LENGTH,fp)!=NULL) {
		pattern[strlen(pattern)-1] = '\0';
/*		fgetc(fp); */
		upsegAddQuery(info,pattern);
	}
}

void upsegAddQuery (upsegQueryInfo *info, char *query)
{
	int nblocks;

	if (info->nstrings==0) {
		info->strings = (char **) malloc (QUERY_BLOCKSIZE*sizeof(char *));
	} else if ((info->nstrings+1)%QUERY_BLOCKSIZE==0) {
		nblocks = (info->nstrings+1)/QUERY_BLOCKSIZE;
		info->strings = (char **) realloc (info->strings
			,sizeof(char *) * (nblocks+1) * QUERY_BLOCKSIZE);
	}
	if (info->strings==NULL) {
		fprintf (stderr,"error: unable to allocate mem for queries!\n");
		exit(1);
	}
	info->strings[info->nstrings] = strdup(query);
	fprintf (stderr,"added pattern %d='%s' for matching\n",info->nstrings
		,info->strings[info->nstrings]);
	info->nstrings++;
}

void upsegAddFile (upsegQueryInfo *info, char *fname)
{
	if (info->nfiles>MAXQUERYFILES) {
		fprintf (stderr,"using fname %d: '%s'\n",info->nfiles,fname);
		fprintf (stderr,"error: only %d files can be given to be searched!\n"
			,MAXQUERYFILES);
		fprintf (stderr,"recompile up_segments_io with a larger value for MAXQUERYFILES\n");
		fprintf (stderr,"or use less files!\n");
		exit(1);
	}
	if (strlen(fname)>MAX_QUERY_FILE_LENGTH) {
		fprintf (stderr,"using fname %d: '%s'\n",info->nfiles,fname);
		fprintf (stderr,"uplib was compiled with MAX_QUERY_FILE_LENGTH=%d!!\n",MAX_QUERY_FILE_LENGTH);
		fprintf (stderr,"use a smaller filename or re-compile uplib!\n");
		exit(1);
	}
	fprintf (stderr,"adding file %d: %s\n",info->nfiles,fname);
	strcpy(info->files[info->nfiles++],fname);
}

static int is_in_range (upsegQueryInfo *info)
{
	if (info->index!=-1)
		return info->index==info->current;
	if (info->first!=-1) {
		if (info->last!=-1) {
			return (info->current>=info->first && info->current<=info->last);
		}
		else
			return info->current>=info->first;
	}
	else {
		if (info->last!=-1)
			return info->current<=info->last;
	}
	return 1;
}


/* a module for matching regular expressions */

static char next_token (char **q)
{
	while ((*q)[0]) {
		switch ((*q)[0]) {
			case '*':
				break;
			default:
				return (*q)[0];
		}
		*q += 1;
	}
	return '\0';
}

static int q_is_star (char *q)
{
	while (q[0]) {
		if (q[0]!='*')
			return 0;
		q++;
	}
	return 1;
}

static int enumerate_chars (int i, char *sub, char c_start, char c_end)
{
	char c;

	for (c=c_start;c<=c_end;c++)
		sub[i++] = c;
	return i;
}

static char *parse_sub (char **query)
{
	static char sub[8192];
	char *q, c_start, c_end, c;
	int i;

	i = 0;
	q = *query+1;
	while (q[1]) {
		switch (c=q[1]) {
			case '-':
				c_start = q[0];
				c_end   = q[2];
				if (c_end=='\0') {
					fprintf (stderr,"error in regular expression '%s'!\n",*query);
					exit(1);
				}
				i = enumerate_chars (i,sub,c_start,c_end);
				q += 1;
				break;
			case ']':
				sub[i++] = q[0];
				sub[i]   = '\0';
				*query = q+1;
				return sub;
			default:
				sub[i++] = q[0];
		}
		q++;
	}
	fprintf (stderr,"error in regular expression '%s'!\n",*query);
	exit(1);
}

static int loe_match (char *s, char *q)
{
	char next_c,*s2;

	while (s[0]) {
		switch (q[0]) {
			case '*':
				if (q[1]=='\0')
					return 1;
				next_c = next_token(&q);
				if (next_c=='?') {
					while (s[0]) {
						if (loe_match(s,q))
							return 1;
						s++;
					}
					return 0;
				}
				while ((s2=strchr(s,next_c))!=NULL) {
					s = s2+1;
					if (loe_match(s,q+1))
						return 1;
				}
				return 0;
			case '\0':
				return 0;
			case '\\':
				q++;
				if (q[0]!=s[0])
					return 0;
				break;
			default:
				if (s[0]!=q[0])
					return 0;
		}
		q++;
		s++;
	}
	if (q_is_star(q))
		return 1;
	else
		return 0;
}

static int loe_match_glob (char *s, char *q)
{
	char next_c,*s2;

	while (s[0]) {
		switch (q[0]) {
			case '*':
				if (q[1]=='\0')
					return 1;
				next_c = next_token(&q);
				if (next_c=='?') {
					while (s[0]) {
						if (loe_match_glob(s,q))
							return 1;
						s++;
					}
					return 0;
				}
				while ((s2=strchr(s,next_c))!=NULL) {
					s = s2+1;
					if (loe_match_glob(s,q+1))
						return 1;
				}
				return 0;
			case '\0':
				return 0;
			case '\\':
				q++;
				if (q[0]!=s[0])
					return 0;
				break;
			case '[':
				s2 = parse_sub(&q);
				while (s2[0]) {
					if (s[0]==s2[0])
						break;
					s2++;
				}
				if (s2[0]=='\0') {
					return 0;
				}
				break;
			case '?':
				break;
			default:
				if (s[0]!=q[0])
					return 0;
		}
		q++;
		s++;
	}
	if (q_is_star(q))
		return 1;
	else
		return 0;
}

static int this_entry_is_allowed (upsegQueryInfo *info, char *name)
{
	int i;

	if (info->use_glob) {
		if (info->nstrings>0) {
			for (i=0;i<info->nstrings;i++) {
				if (loe_match_glob(name,info->strings[i])) {
					info->current++;
					return is_in_range(info);
				}
			}
			return 0;
		}
	} else {
		if (info->nstrings>0) {
			for (i=0;i<info->nstrings;i++) {
				if (loe_match(name,info->strings[i])) {
					info->current++;
					return is_in_range(info);
				}
			}
			return 0;
		}
	}
	info->current++;
	return is_in_range(info);
}

static tUPEntry *get_next_entry (tUPUnipen *pUnipen, upsegQueryInfo *info, char thename[256])
{
	tUPEntry *entry;
	char *name;

	if ((entry=upNextSegment(pUnipen,info->level,0))==NULL) {
		return NULL;
	}
	name = upEntryName(entry);
	strcpy(thename,name);
	free(name);
	if (this_entry_is_allowed(info,thename))
		return entry;
	return get_next_entry(pUnipen,info,thename);
}

static int determine_how_many (tUPUnipen *pUnipen, upsegQueryInfo *info)
{
	tUPEntry *entry;
	char *name;
	int n = 0;

	while ((entry=upNextSegment(pUnipen,info->level,0))!=NULL) {
		name = upEntryName(entry);
		if (this_entry_is_allowed(info,name)) {
			n++;
		}
		free(name);
	}
	return n;
}

static int check_writing_style (tUPUnipen *pUnipen, char *writing_style)
{
	char *wstyle;
	int key_id;

	if (writing_style==NULL)
		return 1;
   if ((key_id=upGetKeywordId(pUnipen,".STYLE"))<0) {
	      fprintf (stderr,"non-existing keyword [%s]... (skipping)\n",writing_style);
			return 0;
	}
	if (pUnipen->NrOfEntries[key_id]!=1) {
		fprintf (stderr,"%d entries for [%s]... (skipping)\n"
			,pUnipen->NrOfEntries[key_id],writing_style);
		return 0;
	}
	wstyle = pUnipen->Entries[key_id][0]->Entry;
	if (strstr(writing_style,"PRINT")!=0) {
		if ( strstr(wstyle,"PRINT") != NULL ) {
			return 1;
		} else {
			fprintf (stderr,"writing style is [%s]... (skipping)\n",wstyle);
			return 0;
		}
	} else 
		if ( strstr(wstyle,writing_style) != NULL ) {
			return 1;
		} else {
			fprintf (stderr,"writing style is [%s]... (skipping)\n",wstyle);
			return 0;
		}
}

static int n_matching_segments (tUPUnipen *pUnipen, char *fname, upsegQueryInfo *query_info)
{
	int n;

	if (!upNextFile(pUnipen,fname))
		return 0;
	if (!check_writing_style(pUnipen,query_info->writing_style))
		return 0;

	/* determine how many segment entries fulfil the requirements */
	n = determine_how_many(pUnipen,query_info);
fprintf (stderr,"%s: %d matches\n",fname,n);
	return n;
}

void upsegInitializeQueryInfo (upsegQueryInfo *info)
{
	info->pd            = _QUERY_PD;
	info->pu            = _QUERY_PU;
	info->width         = _QUERY_WIDTH;
	info->height        = _QUERY_HEIGHT;
	info->xscale        = _QUERY_XSCALE;
	info->yscale        = _QUERY_YSCALE;
	info->ystep         = 1.;
	info->boxsize       = _QUERY_INITIAL_BOXSIZE;
	info->ncolumns      = _QUERY_NCOLUMNS;
	info->fontsize      = _QUERY_FONTSIZE;
	info->pointsize     = -1;
	info->first         = -1;
	info->last          = -1;
	info->index         = -1;
	info->use_glob      =  1;
	info->minpres       = 15.0;
	info->same_scale    =  0; 
	info->margin        = .05;
	info->nstrings      =  0; 
	info->nfiles        =  0; 
	info->nolabel       =  0; 
	info->label_offset  = 0;
	info->label_offsetx = 0;
	info->label_offsety = 0;
	info->page_offsetx  = 40;
	info->page_offsety  = 100;
	strcpy (info->level,_QUERY_LEVEL);
	info->writing_style = NULL;
}

sigSignal **upsegGetSignals (upsegQueryInfo *info, int *nsignals, char ***names)
{
	tUPUnipen *pUnipen;
	tUPEntry *entry;
	sigSignal *sig,**signals;
	char name[256];
	int i,f,n;

	pUnipen = upNewUnipen("unipen.def");
	*nsignals = n = 0;
fprintf (stderr,"looking for items in %d files\n",info->nfiles);
	for (f=0;f<info->nfiles;f++) {
		n += n_matching_segments(pUnipen,info->files[f],info);
	}
	if (n<=0) {
		fprintf (stderr,"no items fulfil your query!\n");
		upDelUnipen(pUnipen);
		return NULL;
	}

	upResetSegments(pUnipen);
	info->current = 0;

	i = 0;
	signals = NULL;
	up_alloc(*names,char *,n);
	for (f=0;f<info->nfiles;f++) {
		if (!upNextFile(pUnipen,info->files[f]))
			continue;
		if (!check_writing_style(pUnipen,info->writing_style))
			continue;
		while ((entry=get_next_entry(pUnipen,info,name))!=NULL) {
			sig = sigEntry2Signal (pUnipen,entry,TIME_EQUI_DIST);
			if (i==0) {
				up_alloc(signals,sigSignal *,1);
			}
			else {
				up_realloc(signals,sigSignal *,i+1);
			}
			signals[i] = sig;
			(*names)[i] = upEntryName(entry);
			i++;
		}
	}
	*nsignals = n;
	return signals;
}

tUPEntry **upsegGetEntries (upsegQueryInfo *info, int *nentries, char ***names)
{
	tUPUnipen *pUnipen;
	tUPEntry *entry,**entries;
	char name[256];
	int i,f,n;

	pUnipen = upNewUnipen("unipen.def");
	info->current = 0;
	i = 0;
	entries = NULL;
	*nentries = 0;
	entries = NULL;
	for (f=0;f<info->nfiles;f++) {
		if (!upNextFile(pUnipen,info->files[f]))
			continue;
		if (!check_writing_style(pUnipen,info->writing_style))
			continue;
		while ((entry=get_next_entry(pUnipen,info,name))!=NULL) {
			if (i==0) {
				up_alloc(entries,tUPEntry *,1);
				up_alloc(*names,char *,1);
			}
			else {
				up_realloc(entries,tUPEntry *,i+1);
				up_realloc(*names,char *,i+1);
			}
			entries[i] = upNewEntry(strdup(entry->Entry),entry->Count);
			(*names)[i] = upEntryName(entry);
			i++;
		}
	}

	n = i;
	if (n<=0) {
		fprintf (stderr,"no items fulfil your query!\n");
	}
	*nentries = n;
	upDelUnipen(pUnipen);
	return entries;
}

tUPEntry **upsegGetEntriesWithStreams (upsegQueryInfo *info
	, int *nentries, char ***names, sigCharStream ***streams, char ***level_names, char **hierarchy)
{
	tUPUnipen *pUnipen;
	tUPEntry *entry,**entries;
	sigCharStream *stream;
	char name[256];
	int i,n,f;

	info->pUnipen = pUnipen = upNewUnipen("unipen.def");
	info->current = 0;
	i = 0;
	entries = NULL;
	for (f=0;f<info->nfiles;f++) {
		if (!upNextFile(pUnipen,info->files[f]))
			continue;
		if (!check_writing_style(pUnipen,info->writing_style))
			continue;
		while ((entry=get_next_entry(pUnipen,info,name))!=NULL) {
			if (i==0) {
				up_alloc(entries,tUPEntry *,1);
				up_alloc(*streams,sigCharStream *,1);
				up_alloc(*names,char *,1);
				if (level_names!=NULL) {
					up_alloc(*level_names,char *,1);
				}
			}
			else {
				up_realloc(entries,tUPEntry *,i+1);
				up_realloc(*streams,sigCharStream *,i+1);
				up_realloc(*names,char *,i+1);
				if (level_names!=NULL) {
					up_realloc(*level_names,char *,i+1);
				}
			}
			entries[i] = upNewEntry(strdup(entry->Entry),entry->Count);
			(*names)[i] = upEntryName(entry);
			if (level_names!=NULL) {
				(*level_names)[i] = upLevelName(entry);
				if (i==0) {
					*hierarchy = strdup((*level_names)[i]);
				} else if (strstr(*hierarchy,(*level_names)[i])==NULL) {
					up_realloc(*hierarchy,char,strlen(*hierarchy)+strlen((*level_names)[i])+1);
					strcat(*hierarchy," ");
					strcat(*hierarchy,(*level_names)[i]);
				}
			}
			stream = sigGetSamples (pUnipen,entry);
			if (stream==NULL)
				break;
			(*streams)[i] = stream;
			i++;
		}
	}

	n = i;
	if (n<=0) {
		fprintf (stderr,"no items fulfil your query!\n");
	}
	*nentries = n;
	return entries;
}
