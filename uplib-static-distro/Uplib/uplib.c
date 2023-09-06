/**************************************************************************
*                                                                         *
*  UNIPEN PROJECT (uplib.c)                                               *
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
#include <sys/types.h>
#include <sys/stat.h>
#include "uplib.h"

#ifndef UPLIB_VERSION
#define UPLIB_VERSION "uplib version 3.03 April 1997"
#endif

extern double strtod (const char *,char **); /* not found on all platforms */


/*************************************************************************/
/*                                                                       */
/* static routines used intern in uplib.c                                */
/*                                                                       */
/* upAddString                                                           */
/* upChangeMemory                                                        */
/* upDelMemory                                                           */
/* upDelString                                                           */
/* upMatch                                                               */
/* upSetUpIncludePaths                                                   */
/*                                                                       */
/*************************************************************************/

static char  *upAddString         (char *, char *);
static void  *upChangeMemory      (void *, size_t);
static void  *upDelMemory         (void *);
static char  *upDelString         (char *);
static upBool upMatch             (char *, char *);
static int   upSetUpIncludePaths (tUPUnipen *);

static char *upAddString (char *_String1_, char *_String2_)
{
	/* special cases */
	if (_String1_ == NULL)
		return (upNewString (_String2_));

	if (_String2_ == NULL)
		return (_String1_);

	/* reallocate memory and append string */
	_String1_ = (char *) upChangeMemory (_String1_, strlen (_String1_)
		+ strlen (_String2_) + 1);
	strcat (_String1_, _String2_);

	return (_String1_);
}

/*************************************************************************/
/*                                                                       */
/* memory pool                                                           */
/*                                                                       */
/*   Reallocation of memory is *very* slow on the HP Apollo!             */
/*   Therefore we'll use a memory pool (see realloc() in this file)      */
/*                                                                       */
/*   By trial and error we've found the following optima                 */
/*                                                                       */
/*   HP Apollo:     memory pool = 768                                    */
/*   DecStation:    memory pool = 1 - 10,000                             */
/*   SparcStation:  memory pool = 1 - 10,000                             */
/*                                                                       */
/*************************************************************************/

#define cMemoryPool                    768


static void *upChangeMemory (void *_pMemory_, size_t _Size_)
{
	if (_pMemory_ == NULL)
		return (upNewMemory (_Size_));

	if (_Size_ == 0)
		return (upDelMemory (_pMemory_));

	if ((_pMemory_ = (void *) realloc (_pMemory_, cMemoryPool * (1 + (_Size_ /cMemoryPool)))) == NULL) {
		fprintf (stderr,"%s",errOutOfMemory);
		exit (1);
	}

	return (_pMemory_);
}


static void *upDelMemory (void *_pMemory_)
{
	/* nothing to do */
	if (_pMemory_ == NULL)
		return (NULL);
	/* free memory */
	free (_pMemory_);
	_pMemory_ = NULL;

	return (_pMemory_);
}


static char *upDelString (char *_String_)
{
	/* free memory */
	return ((char *) upDelMemory (_String_));
}


static upBool upMatch (char *_String1_, char *_String2_)
{
	char *TmpString1, *TmpString2;
	int String1Nr, String2Nr;
	upBool Match;

	/* _String1_ == NULL or _String2_ == NULL */
	if ((_String1_ == NULL) || (_String2_ == NULL))
		return ((_String1_ == NULL) && (_String2_ == NULL));

	/* _String1_ == "" and _String2_ == "" */
	if ((strlen (_String1_) == 0) && (strlen (_String2_) == 0))
		return (cTrue);

	/* _String2_ == "*---" */
	if (_String2_ [0] == '*')
	{
		for (String1Nr = 0; String1Nr <= strlen (_String1_); String1Nr = String1Nr + 1)
		{
			if (upMatch (&_String1_ [String1Nr], &_String2_ [1]))
				return (cTrue);
		}

		return (cFalse);
	}

	/* _String1_ == "[\t\n\r ]---" or _String2_ == "[\t\n\r ]---" */
	if (mIsScheider(_String1_[0]) || mIsScheider(_String2_[0]))
	{
		if (!mIsScheider(_String1_[0]) || !mIsScheider(_String2_[0]))
			return (cFalse);
		/* skip '\t', '\n', '\r', ' ' */
		for (String1Nr = 0; String1Nr < strlen (_String1_); String1Nr = String1Nr + 1)
		{
			if (!mIsScheider(_String1_[String1Nr]))
				break;
		}

		/* skip '\t', '\n', '\r', ' ' */
		for (String2Nr = 0; String2Nr < strlen (_String2_); String2Nr = String2Nr + 1)
		{
			if (!mIsScheider(_String2_[String2Nr]))
				break;
		}
		return (upMatch (&_String1_ [String1Nr], &_String2_ [String2Nr]));
	}


	/* _String1_ == ""---"---"  or _String2_ == ""---"---" */
	if ((_String1_ [0] == '"') || (_String2_ [0] == '"'))
	{
		if ((_String1_ [0] != '"') || (_String2_ [0] != '"'))
			return (cFalse);

		/* search for ending '"' */
		for (String1Nr = 1; String1Nr < strlen (_String1_); String1Nr = String1Nr + 1)
		{
			/* found ending '"'? */
			if (_String1_ [String1Nr] == '"')
				break;
		}

		/* no ending '"'? */
		if (String1Nr == strlen (_String1_))
			return (cFalse);

		/* search for ending '"' */
		for (String2Nr = 1; String2Nr < strlen (_String2_); String2Nr = String2Nr + 1)
		{
			/* found ending '"'? */
			if (_String2_ [String2Nr] == '"')
				break;
		}

		/* no ending '"'? */
		if (String2Nr == strlen (_String2_))
			return (cFalse);

		TmpString1 = upNewString (&_String1_ [1]);
		TmpString2 = upNewString (&_String2_ [1]);

		TmpString1 [String1Nr - 1] = '\0';
		TmpString2 [String2Nr - 1] = '\0';

		Match = upMatch (TmpString1, TmpString2);

		TmpString1 = upDelString (TmpString1);
		TmpString2 = upDelString (TmpString2);

		return (Match && upMatch (&_String1_ [String1Nr + 1], &_String2_ [String2Nr + 1]));
	}

	/* _String1_ == "" or _String2_ == "" */
	if ((strlen (_String1_) == 0) || (strlen (_String2_) == 0))
		return (cFalse);

	/* _String2_ == "?---" */
	if (_String2_ [0] == '?')
	{
		return (upMatch (&_String1_ [1], &_String2_ [1]));
	}

	return ((_String1_ [0] == _String2_ [0]) && upMatch (&_String1_ [1], &_String2_ [1]));
}


static int upSetUpIncludePaths (tUPUnipen *pUnipen)
{
	char *Path;
	int n;
	char include_dummy[512];


	/* panic */
	if (pUnipen == NULL) {
		fprintf (stderr,"PANIC: upSetUpIncludePaths\n");
		return 0;
	}

	/* try to find upIncludePath */

	if ((Path=getenv(cUnipenIncludePath))!=NULL ) {
		sscanf(Path,"%s",include_dummy);
		up_alloc(pUnipen->upIncludePaths,char *,1);
		up_alloc(pUnipen->upIncludePaths[0],char,strlen(include_dummy)+1);
		strcpy (pUnipen->upIncludePaths[0],include_dummy);
		Path += strlen(include_dummy);
		n = 1;
		while (Path[0]!='\0') {
			Path += 1;
			if (sscanf(Path,"%s",include_dummy)==1) {
				up_realloc(pUnipen->upIncludePaths,char *,n+1);
				up_alloc(pUnipen->upIncludePaths[n],char,strlen(include_dummy)+1);
				strcpy (pUnipen->upIncludePaths[n],include_dummy);
				Path += strlen(include_dummy);
				n++;
			}
			else {
				break;
			}
		}
		pUnipen->NrIncludePaths = n;
	}
	else {
		fprintf (stderr,"No UNIPEN include paths, environment variable %s empty!\n",cUnipenIncludePath);
		fprintf (stderr,"You may solve this by setting the variable as:\n");
		fprintf (stderr,"setenv %s \". .INHERIT\"\n",cUnipenIncludePath);
		exit(1);
	}
	return 1;
}



/*************************************************************************/
/*                                                                       */
/* Some extra debug tools for the developer                              */
/*                                                                       */
/*************************************************************************/

void *upNewMemory (size_t _Size_)
{
	void *pMemory;

	/* nothing to do */
	if (_Size_ == 0)
		return (NULL);

	/* allocate memory */
	if ((pMemory = (void *) calloc (1,_Size_)) == NULL) {
		fprintf (stderr,"%s",errOutOfMemory);
		exit (1);
	}

	return (pMemory);
}

char *upNewString (char *_String_)
{
	char *String;

	/* nothing to do */
	if (_String_==NULL)
		return (NULL);
	/* allocate memory and copy string */
	String = (char *) upNewMemory (strlen (_String_) + 1);
	strcpy (String, _String_);
	return (String);
}


char *upNthString (char *s, int n)
{
	/* skip until the first non-scheider */
	while (mIsScheider(*s)&&*s!='\0') s++;
	/* skip until n==0 */
	while (n!=0) {
		/* skip until the first scheider */
		while (!mIsScheider(*s)&&*s!='\0') s++;
		if (*s=='\0') return NULL;
		/* and skip until the first non-scheider */
		while (mIsScheider(*s)&&*s!='\0') s++;
		if (*s=='\0') return NULL;
		n--;
	}
	/* skip until the first non-scheider */
	while (mIsScheider(*s)&&*s!='\0') s++;
	return s;
}

int upFileSize (char *fname)
{
	struct stat f_status_info;

	stat(fname, &f_status_info);
	return(f_status_info.st_size);
}

FILE *upOpenIncludeFile (tUPUnipen *pUnipen,char *fname, char *mode, int *nbytes)
{

	FILE *fp;
	int i;
	char dummyfname[512];
	char *rel_path = pUnipen->cur_path;

	if ((fp=fopen(fname,mode))!=NULL) {
		*nbytes = upFileSize (fname);
		fprintf (stderr,"opened '%s' (%d bytes)\n",fname,*nbytes);
		return fp;
	}
	
	for (i=0; i<pUnipen->NrIncludePaths; i++) {
		if (strcmp(pUnipen->upIncludePaths[i],cIncludeInheritanceKey)==0) {
			sprintf (dummyfname,"%s/%s",rel_path,fname);
		}
		else {
			sprintf (dummyfname,"%s/%s",pUnipen->upIncludePaths[i],fname);
		}
		if ((fp=fopen(dummyfname,mode))!=NULL) {
			*nbytes = upFileSize (dummyfname);
			fprintf (stderr,"opened '%s' (%d bytes)\n",dummyfname,*nbytes);
			return fp;
		}
	}
	fprintf (stderr,".INCLUDE unable to open file `%s'!\n",fname);
	if (pUnipen->NrIncludePaths==0)
		fprintf (stderr,"maybe you should set your %s environment variable\n",cUnipenIncludePath);
	else {
		fprintf (stderr,"maybe you should expand your %s environment variable\n",cUnipenIncludePath);
		fprintf (stderr,"its current value is: %s\n",getenv(cUnipenIncludePath));
	}
	fprintf (stderr,"a proper setting would be something like: \". .INHERIT ~/unipen-include\"\n");
	return NULL;
}


char *upConstructDefinitionFile (char *deffile)
{
	FILE *fp;
	char *fname;

	if ((deffile!=NULL) && (strcmp(deffile,"") != 0)) {
		if ((fp=fopen(deffile,"rb"))!=NULL) {
			fclose(fp);
			return upNewString(deffile);
		}
	}
	if ((fname=getenv(cUnipenDefinition))!=NULL) {
		if ((fp=fopen(fname,"rb"))!=NULL) {
			fclose(fp);
			return upNewString (fname);
		}
		if ((fp=fopen(cDefFile,"rb"))!=NULL) {
			fclose(fp);
			return upNewString(cDefFile);
		}
		else {
			fprintf (stderr,"unable to open any unipen.def file!!\n try setting environment variable %s\n",cUnipenDefinition);
			exit(1);
		}
	}
	else {
		if ((fp=fopen(cDefFile,"rb"))!=NULL) {
			fclose(fp);
			return upNewString(cDefFile);
		}
		else {
			fprintf (stderr,"unable to open any unipen.def file!!\n try setting environment variable %s\n",cUnipenDefinition);
			return NULL;
		}
	}
}

static void upCleanKeywords (tUPUnipen *pUnipen)
{
	/* in upRequireKeyword, entry administration was allocated but not set to zero */
	int nbytes;
	int i;

	nbytes = pUnipen->NrOfKeywords*sizeof(int);
	memset (pUnipen->NrOfEntries,0,nbytes);
	nbytes = pUnipen->NrOfKeywords*sizeof(tUPEntry **);
	memset (pUnipen->Entries,0,nbytes);
	for (i=0;i<pUnipen->NrOfKeywords;i++)
		pUnipen->CurrOffsetEntries [i] = -1;
}


void upResetUnipen (tUPUnipen *pUnipen)
{
	/* panic */
	if (pUnipen == NULL) {
		fprintf (stderr,"PANIC: upResetUnipen\n");
		return;
	}
	/* short cuts */
	pUnipen->SetId        = -1;
	pUnipen->HierarchyId  = -1;
	pUnipen->CoordId      = -1;
	pUnipen->SegmentId    = -1;
	pUnipen->IncludeId    = -1;
	pUnipen->PenDownId    = -1;
	pUnipen->PenUpId      = -1;
	pUnipen->PenLapseId   = -1;
	pUnipen->SampleRateId = -1;
	upCleanKeywords(pUnipen);
	/* streams */
	pUnipen->last_pen_stream = -1;
}

int upCheckMandatoryKeywords (tUPUnipen *pUnipen, int exit_on_failure)
{
	int i,id;
	char *keyword;

	for (i=0;i<pUnipen->NrOfMandatoryKeywords;i++) {
		keyword = pUnipen->MandatoryKeywords[i];
		fprintf (stderr,"checking existence of keyword[%s]\n",keyword);
		id = upGetKeywordId (pUnipen,keyword);
		if (id<0) {
			fprintf (stderr,"Keyword [%s] non existing but mandatory!!!\n",keyword);
			if (exit_on_failure)
				exit(1);
			else
				return 0;
		}
		if (pUnipen->NrOfEntries[id]<=0) {
			fprintf (stderr,"Keyword [%s] non existing but mandatory!!!\n",keyword);
			if (exit_on_failure)
				exit(1);
			else
				return 0;
		}
	}
	return 1;
}

void upRequireKeyword (tUPUnipen *pUnipen, char *keyword, int mandatory)
{
	int i, already_exists;
	char *keyptr; /* points into Keywords array, so only one copy per keyword */

	keyptr = NULL;
	already_exists = 0;
	for (i=0;i<pUnipen->NrOfKeywords;i++)
		if (strcmp(pUnipen->Keywords[i],keyword)==0) { /* already exists */
			already_exists = 1;
			keyptr = pUnipen->Keywords[i];
			break;
		}
	if (!already_exists) {
		/* increase nr of keywords */
		pUnipen->NrOfKeywords++;
		/* reallocate keywords and allocate entries for each keyword */
		pUnipen->Keywords           = (char **) upChangeMemory (pUnipen->Keywords
			, pUnipen->NrOfKeywords * sizeof (char *));
		pUnipen->NrOfEntries        = (int *) upChangeMemory (pUnipen->NrOfEntries
			, pUnipen->NrOfKeywords * sizeof (int));
		pUnipen->CurrOffsetEntries  = (int *) upChangeMemory (pUnipen->CurrOffsetEntries
			, pUnipen->NrOfKeywords * sizeof (int));
		pUnipen->Entries            = (tUPEntry ***) upChangeMemory (pUnipen->Entries
			, pUnipen->NrOfKeywords * sizeof (tUPEntry **));
		/* copy keyword */
		pUnipen->Keywords[pUnipen->NrOfKeywords-1] = upNewString (keyword);
		keyptr = pUnipen->Keywords[pUnipen->NrOfKeywords-1];
	}

	if (mandatory) {
		already_exists = 0;
		for (i=0;i<pUnipen->NrOfMandatoryKeywords;i++)
			if (strcmp(pUnipen->MandatoryKeywords[i],keyword)==0) {
				already_exists = 1;
				break;
			}
		if (!already_exists) {
			if (keyptr==NULL) {
				fprintf (stderr,"strange error occurred while inserting mandatory keyword [%s]!!\n",keyword);
				exit(1);
			}
			fprintf (stderr,"marking keyword [%s] as mandatory at %d....\n"
				,keyptr,pUnipen->NrOfMandatoryKeywords);
			pUnipen->NrOfMandatoryKeywords++;
			pUnipen->MandatoryKeywords =
				(char **) upChangeMemory (pUnipen->MandatoryKeywords
					, pUnipen->NrOfMandatoryKeywords * sizeof (char *));
			pUnipen->MandatoryKeywords[pUnipen->NrOfMandatoryKeywords-1] =
				keyptr;
			fprintf (stderr,"MARKED [%s] MANDATORY\n",pUnipen->MandatoryKeywords[pUnipen->NrOfMandatoryKeywords-1]);
		}
	}
}

tUPUnipen *upNewUnipen (char *_DefFile_)
{
	tUPUnipen *pUnipen;
	char keyword[8192],key[8192],line[8192];
	char *unipen_file;
	FILE *fp;

	/* allocate memory */
	pUnipen = (tUPUnipen *) upNewMemory (sizeof (tUPUnipen));

	/* scan keywords */
	if ((unipen_file=upConstructDefinitionFile(_DefFile_))==NULL) {
		return NULL;
	}
	if ((fp=fopen(unipen_file,"r"))==NULL) {
		fprintf (stderr,"\nupNewUnipen: unable to open unipenfile '%s' correctly!!\n",unipen_file);
		fprintf (stderr,"Maybe the file does not exist?\n");
		return NULL;
	}
	while (fgets(line,1024,fp)!=NULL) {
		if (sscanf(line,"%s",key)==1) {
			if (strcmp (key, keyKeyword) == 0) { /* a possible .KEYWORD definition */
				if (sscanf(line,"%s%s",key,keyword)!=2) {
					fprintf (stderr,"error in unipenfile %s:\n%s\n",unipen_file,line);
					return NULL;
				}
				if ((strcmp(keyword,keyKeyword)!=0)&&(strcmp(keyword,keyReserve)!=0)) {
					upRequireKeyword (pUnipen,keyword,0);
				}
			}
		}
	}
	fclose(fp);
	free(unipen_file);

	/* reset UNIPEN administration */
	upResetUnipen(pUnipen);

	/* set keyword short cuts */
	pUnipen->SetId        = upGetKeywordId (pUnipen, keySet       );
	pUnipen->HierarchyId  = upGetKeywordId (pUnipen, keyHierarchy );
	pUnipen->CoordId      = upGetKeywordId (pUnipen, keyCoord     );
	pUnipen->SegmentId    = upGetKeywordId (pUnipen, keySegment   );
	pUnipen->IncludeId    = upGetKeywordId (pUnipen, keyInclude   );
	pUnipen->SampleRateId = upGetKeywordId (pUnipen, keySampleRate);
	pUnipen->PenDownId    = upGetKeywordId (pUnipen, keyPenDown   );
	pUnipen->PenUpId      = upGetKeywordId (pUnipen, keyPenUp     );
	pUnipen->PenLapseId   = upGetKeywordId (pUnipen, keyPenLapse  );

	if (!upSetUpIncludePaths(pUnipen)) {
		return NULL;
	}
	return (pUnipen);
}


tUPUnipen *upDelUnipen (tUPUnipen *pUnipen)
{
	int i,j,KeywordId;
	tUPEntry ***entries;

	/* panic */
	if (pUnipen == NULL) {
		fprintf (stderr,"PANIC: upDelUnipen\n");
		return NULL;
	}

	entries=pUnipen->Entries;
	if (pUnipen->nunipens!=0) {
		up_free(pUnipen->cur_file_open);
		up_free(pUnipen->cur_path);
		for (i=0;i<pUnipen->nunipens;i++)
			up_free(pUnipen->file_contents[i]);
		up_free(pUnipen->file_contents);
		pUnipen->nunipens = 0;
	}
	/* delete previous entries */
	for (i=0;i<pUnipen->NrOfKeywords;i++) {
		for (j=0;j<pUnipen->NrOfEntries[i];j++)
			up_free(entries[i][j]);
		pUnipen->NrOfEntries[i] =  0;
        pUnipen->CurrOffsetEntries[i] = -1;
	}
	/* free all pointers and reset all counts */
	for (i=0;i<pUnipen->NrOfLevels;i++)
		up_free(pUnipen->Levels[i]);
	up_free(pUnipen->Levels);
	up_free(pUnipen->NrOfSegmentsInLevel);
	for (i=0;i<pUnipen->NrOfCoords;i++)
		up_free(pUnipen->Coords[i]);
	up_free(pUnipen->Coords);
	pUnipen->NrOfLevels          = 0;
	pUnipen->NrOfCoords          = 0;
	pUnipen->TotalNrOfEntries    = 0;
	pUnipen->last_pen_stream     = -1;
	pUnipen->nvalid_streams      = 0;
	pUnipen->NrOfPenStreams      = 0;
	mBoundedDelineation(pUnipen) = mUnknownDelineation(pUnipen) = 0;
	up_free(pUnipen->entry_sequence);
	up_free(pUnipen->stream_sequence);
	up_free(pUnipen->valid_streams);

	/* include paths */
	if (pUnipen->NrIncludePaths>0) {
		for (i=0;i<pUnipen->NrIncludePaths;i++)
			free (pUnipen->upIncludePaths[i]);
		up_free(pUnipen->upIncludePaths);
	}

	/* delete keywords */
	for (KeywordId = 0; KeywordId < pUnipen->NrOfKeywords; KeywordId = KeywordId + 1)
		pUnipen->Keywords [KeywordId] = upDelString (pUnipen->Keywords [KeywordId]);
	pUnipen->Keywords = (char **) upDelMemory (pUnipen->Keywords);
	pUnipen->MandatoryKeywords = (char **) upDelMemory (pUnipen->MandatoryKeywords);
	pUnipen->NrOfKeywords = 0;
	pUnipen->NrOfMandatoryKeywords = 0;

	/* entries */
	pUnipen->TotalNrOfEntries  = 0;
	pUnipen->NrOfEntries       = (int        *) upDelMemory (pUnipen->NrOfEntries      );
	pUnipen->CurrOffsetEntries = (int        *) upDelMemory (pUnipen->CurrOffsetEntries);
	pUnipen->Entries           = (tUPEntry ***) upDelMemory (pUnipen->Entries          );
	pUnipen->entry_sequence    = (tUPEntry **)  upDelMemory (pUnipen->entry_sequence    );
	pUnipen->stream_sequence   = (tUPStream *)  upDelMemory (pUnipen->stream_sequence    );
	pUnipen->valid_streams     = (int *)        upDelMemory (pUnipen->valid_streams);

	/* free memory */
	pUnipen = (tUPUnipen *) upDelMemory (pUnipen);

	return (pUnipen);
}

char *upLevelName (tUPEntry *entry)
{
	char lev_buf[1024];

	if (sscanf(entry->Entry,".SEGMENT %s",lev_buf)!=1) {
		fprintf (stderr,"unable to detect .SEGMENT LEVEL in (%s)!\n",entry->Entry);
		return NULL;
	}
	return strdup(lev_buf);
}

char *upEntryName (tUPEntry *entry)
{
	char *result, *ptr;
	static char *unknown_name = "????";
	int i;
	
	/* Assumptions are: ptr[0] = '"' and 
	                    ptr[strlen(ptr)-1]= '"' */

        ptr = strchr(entry->Entry,'"');

	if (ptr==NULL) {
		up_alloc(result,char,8);
		strcpy (result,"UNKNOWN");
	}
	else {
		i = strlen(ptr) - 1; /* start from tail and skip whitespace */
		while(i >= 1 && (unsigned int) ptr[i] <= 32) {
			--i;
		}
		if(i < 1) {
			fprintf(stderr,"upEntryName() UNIPEN syntax error empty string in %s\n", ptr);
			exit(1);
		}
		if(ptr[i] != '"') {
			fprintf(stderr,"upEntryName() UNIPEN syntax, missing '\"' in %s\n", ptr);
			return strdup(unknown_name);
		}
		up_alloc(result,char,i+1); /* Paranoia byte added */
		strcpy(result,ptr+1); /* Start after '"' */
		result[i-1] = '\0';   /* End before '"' (i is leftshifted 1) */
	}
	return result;
}

extern int upEntryQuality (tUPEntry *entry)
{
	char quality[64];

	/* if quality given, it must be fourth element of entry */
	if (sscanf(entry->Entry,"%*s%*s%*s%s",quality)!=1)
		return Q_UNKNOWN;
	if (strcmp(quality,qUnknown)==0)
		return Q_UNKNOWN;
	if (strcmp(quality,qBad)==0)
		return Q_BAD;
	if (strcmp(quality,qOk)==0)
		return Q_OK;
	if (strcmp(quality,qGood)==0)
		return Q_GOOD;
	if (strcmp(quality,qLbl)==0)
		return Q_LBL;
	if (strcmp(quality,qLbr)==0)
		return Q_LBR;
	if (strcmp(quality,qLbs)==0)
		return Q_LBS;
	fprintf (stderr,"unknown quality `%s'!\n",quality);
	return Q_UNKNOWN;
}

extern float upEntryFloatQuality (tUPEntry *entry)
{
	char quality[64];
	char *ptr1,*ptr2;
	float f;

	/* if quality given, it must be fourth element of entry */
	if (sscanf(entry->Entry,"%*s%*s%*s%s",quality)!=1)
		return -1.0;
	if ((ptr1=strchr(quality,'('))==NULL)
		return -1.0;
	if ((ptr2=strchr(ptr1,')'))==NULL)
		return -1.0;
	ptr2[0] = '\0';
	if (sscanf(ptr1+1,"%f",&f)!=1)
		return -1.0;
	return f;
}


int upNSamples (char *scan)
{
	int n = 0;

	while ((scan=strchr(scan,'\n'))!=NULL) {
		scan++;
		n++;
	}
	return n;
}

/* for UNKNOWN DELINEATIONS: upSearchNextSegmentEntry
 *   - searches for a segment with a certain level [0-level]
 *   - uses int upCurLevel (char *level) to determine the level
*/

int upMatchLevel (char *lev1, char *lev2)
{
	/* returns 1 if levels match, and if theye are CHAR or CHARACTER */
	if (strcmp(lev1,lev2)==0)
		return 1;
	if (strcmp(lev1,"CHAR")==0 && strcmp(lev2,"CHARACTER")==0 )
		return 1;
	if (strcmp(lev2,"CHAR")==0 && strcmp(lev1,"CHARACTER")==0 )
		return 1;
	return 0;
}

int upCurLevel (tUPUnipen *pUnipen, char *level)
{
	int i;

	if (strcmp(level,"*")==0)
		return UP_ANY_LEVEL;
	for (i=0;i<pUnipen->NrOfLevels;i++)
		if (strcmp(pUnipen->Levels[i],level)==0)
			return i;
	/* ok, level not found, maybe it is CHAR
		and we are looking for CHARACTER? */
	if (strcmp(level,"CHAR")==0) {
		for (i=0;i<pUnipen->NrOfLevels;i++)
			if (strcmp(pUnipen->Levels[i],"CHARACTER")==0)
				return i;
	}
	if (strcmp(level,"CHARACTER")==0) {
		for (i=0;i<pUnipen->NrOfLevels;i++)
			if (strcmp(pUnipen->Levels[i],"CHAR")==0)
				return i;
	}
	fprintf (stderr,"PANIC! level %s not found in HIERARCHY\n",level);
	fprintf (stderr,"levels found in %s:",pUnipen->cur_file_open);
	for (i=0;i<pUnipen->NrOfLevels;i++)
		fprintf (stderr," [%s]",pUnipen->Levels[i]);
	fprintf (stderr,"\n");
	return -1;
}

void upSetLastStreamnr(tUPUnipen *pUnipen, int cur_cnt)
{
	int i;

	pUnipen->last_pen_stream = -1;
	for (i=0;i<pUnipen->NrOfPenStreams-1;i++) {
		if (pUnipen->stream_sequence[i].entry->Count>cur_cnt) {
			return;
		}
		pUnipen->last_pen_stream++;
	}
}

int upSynchronizeSegmentOffsetAndCount (tUPUnipen *pUnipen, tUPEntry *entry)
{
	int cur_cnt,cnt = entry->Count;
	int offst = mCurrOffsetSegment(pUnipen);
	tUPEntry **entries = pUnipen->Entries[pUnipen->SegmentId];

	if (mBoundedDelineation(pUnipen)) {
		return 1;
	}

	if (offst<0)
		offst = 0;

	cur_cnt = entries[offst]->Count;
	if (cur_cnt==cnt)
		return 1;
	if (cur_cnt<cnt) {
		while (cur_cnt!=cnt) {
			if (offst>=mNrOfSegments(pUnipen)-1) {
				fprintf (stderr,"while synchronizing %s: count %d not found!\n",entry->Entry,cnt);
				return 0;
			}
			offst++;
			cur_cnt = entries[offst]->Count;
		}
	} else {
		while (cur_cnt!=cnt) {
			if (offst<=0) {
				fprintf (stderr,"while synchronizing %s: count %d not found!\n",entry->Entry,cnt);
				return 0;
			}
			offst--;
			cur_cnt = entries[offst]->Count;
		}
	}
	mCurrOffsetSegment(pUnipen) = offst;
	upSetLastStreamnr(pUnipen,cnt);
	return 1;
}

tUPEntry *upSearchNextSegmentEntry (tUPUnipen *pUnipen, char *level, tUPEntry *Entry)
{
	int i;
	tUPEntry **entries = pUnipen->Entries[pUnipen->SegmentId];
	int curlevel = upCurLevel(pUnipen,level);
	char nextlevel[256];

        if(Entry == NULL) {
	   if ((i=mCurrOffsetSegment(pUnipen))<0) {
	       i = 1;
	   } else {
	       i++;
	   }
	} else {
	   for (i = 0;i<mNrOfSegments(pUnipen);i++) {
	   	if(Entry == entries[i]) break;
	   }
	   ++i;
	}

	if (curlevel==UP_ANY_LEVEL) {
		if (i>=mNrOfSegments(pUnipen))
			return NULL;
		return entries[i];
	}

	for (;i<mNrOfSegments(pUnipen);i++) {
		if (sscanf(entries[i]->Entry,".SEGMENT%s",nextlevel)==1) {
			if (upCurLevel(pUnipen,nextlevel)<=curlevel) {
				return entries[i];
			}
		}
	}
	return NULL;
}

tUPEntry *upSearchNextSegment (tUPUnipen *pUnipen, char *level)
{
	char *Search1, *Search2;
	int i;
	tUPEntry **entries = pUnipen->Entries[pUnipen->SegmentId];

	Search1 = upAddString (upAddString(upNewString(keySegment)," "),level);
	Search2 = upAddString (upAddString(upAddString(upNewString(keySegment)," "),level)," *");

	if ((i=mCurrOffsetSegment(pUnipen))<0)
		i = 0;
	else
		i++;

	for (;i<mNrOfSegments(pUnipen);i++)
		if (upMatch(entries[i]->Entry, Search1)
				|| upMatch (entries[i]->Entry, Search2)) {
			return entries[i];
		}
	return NULL;
}

tUPEntry *upNextSegment (tUPUnipen *pUnipen, char *level, int do_reset)
{
	int i;
	tUPEntry **entries = pUnipen->Entries[pUnipen->SegmentId];
	int lev = upCurLevel(pUnipen,level);
	char nextlev[256];

	if (lev!=UP_ANY_LEVEL&&pUnipen->cur_level!=lev && do_reset) {
		upResetSegments(pUnipen);
		pUnipen->cur_level = lev;
	}

	if (mCurrOffsetSegment(pUnipen)<0)
		mCurrOffsetSegment(pUnipen) = 0; /* take the first */
	else
		mCurrOffsetSegment(pUnipen)++;

	if (lev==UP_ANY_LEVEL) {
		i = mCurrOffsetSegment(pUnipen);
		if (i>=mNrOfSegments(pUnipen))
			return NULL;
		return entries[mCurrOffsetSegment(pUnipen)];
	}

	for (i=mCurrOffsetSegment(pUnipen);i<mNrOfSegments(pUnipen);i++)
		if (entries[i]->Entry!=NULL) {
			if (sscanf(entries[i]->Entry,".SEGMENT%s",nextlev)==1) {
				if (upMatchLevel(nextlev,level)==1) {
					mCurrOffsetSegment(pUnipen) = i;
					return entries[i];
				}
			}
		}
	return NULL;
}


char *upGetArgument (tUPUnipen *pUnipen, char *_Keyword_, int give_error)
{
	char *Argument;

	int KeywordId;
	int offst;

	/* panic */
	if (pUnipen == NULL) {
		fprintf (stderr,"PANIC: upGetArgument\n");
		return NULL;
	}
	KeywordId = upGetKeywordId (pUnipen, _Keyword_);
	if (KeywordId<0)
		return NULL;
	offst = pUnipen->CurrOffsetEntries [KeywordId];
	if ( offst != -1) {
		Argument = pUnipen->Entries [KeywordId][offst]->Entry;
	}
	else { /* try the first */
		if (pUnipen->Entries [KeywordId]==NULL) {
			if (give_error) {
				fprintf (stderr,"keyword entry for '%s' non-existing in UNIPEN file!\n"
					,_Keyword_);
			}
			return NULL;
		}
		if (pUnipen->Entries [KeywordId][0]!=NULL) {
			Argument = pUnipen->Entries [KeywordId][0]->Entry;
		}
		else {
			if (give_error) {
				fprintf (stderr,"keyword entry for '%s' non-existing in UNIPEN file!\n"
					,_Keyword_);
			}
			return NULL;
		}
	}
	return (Argument);
}

int upGetKeywordId (tUPUnipen *pUnipen, char *_Keyword_)
{
	int KeywordId;

	/* panic */
	if (pUnipen == NULL) {
		fprintf (stderr,"PANIC: upGetKeywordId\n");
		return -1;
	}

	/* search for keyword */
	for (KeywordId = 0; KeywordId < pUnipen->NrOfKeywords; KeywordId = KeywordId + 1)
	{
		/* found keyword? */
		if (strcmp (pUnipen->Keywords [KeywordId], _Keyword_) == 0)
			break;
	}

	/* keyword not found? */
	if (KeywordId == pUnipen->NrOfKeywords) {
		fprintf (stderr,errInvalidKeyword,pUnipen->cur_file_open, _Keyword_);
		return -1;
	}
	return (KeywordId);
}

char *NextEntry (char *stream, int *offst)
{
	char *entry = stream;

	if (stream[0]=='\0')
		return NULL;
	while (*stream) {
		(*offst)++;
		if (stream[0]=='\r') {
			if (stream[1]=='\n'&&stream[2]=='.') {
				stream[0] = '\0';
				(*offst)++;
				return entry;
			}
			else {
				stream[0] = ' ';
			}
		}
		if (stream[0]=='\n'&& (stream[1]=='.'||stream[1]=='\0')) {
			stream[0] = '\0';
			if (entry[0]=='\n'||entry[0]=='\0') { /* we don't have a valid entry yet */
				stream++;
				(*offst)++;
				entry = stream;
			}
			else
				return entry;
		}
		stream++;
	}
	return entry;
}

tUPEntry *upNewEntry (char *Entry, int Count)
{
	tUPEntry *entry;
	
	up_alloc(entry,tUPEntry,1);
	entry->Entry = Entry;
	entry->Count = Count;
	return entry;
}

int is_a_version_entry (char *entry)
{
	char keyword[256];

	if (sscanf(entry,"%s",keyword)!=1) {
		fprintf (stderr,"invalid keyword entry:\n   %s\n",entry);
		return 0;
	}
	return (strcmp(keyword,".VERSION")==0);
}

/* To determine whether we have a UNIPEN file here, I require a .VERSION entry,
 * unless a .INCLUDE preceeds all other entries. In the latter case, the decision
 * to find out whether we have UNIPEN is postponed. (Louis Vuurpijl November 1997
*/
int upAdministrateEntry (tUPUnipen *pUnipen, char *entry, int *new_entrynr, int count)
{
	char keyword[256];
	int entrynr,keyword_id;

	if (sscanf(entry,"%s",keyword)!=1) {
		fprintf (stderr,"invalid keyword entry:\n   %s\n",entry);
		return -1;
	}
	keyword_id = upGetKeywordId(pUnipen,keyword);
	if (keyword_id<0) {
		fprintf (stderr,"invalid keyword entry:\n   %s\n",entry);
		return -1;
	}
	entrynr = (pUnipen->NrOfEntries [keyword_id])++;
	if (entrynr==0) {
		up_alloc(pUnipen->Entries[keyword_id],tUPEntry *,1);
	} else {
		up_realloc(pUnipen->Entries[keyword_id],tUPEntry *,entrynr+1);
	}
	if (count==0) {
		up_alloc(pUnipen->entry_sequence,tUPEntry *,1);
	} else {
		up_realloc(pUnipen->entry_sequence,tUPEntry *,count+1);
	}
	pUnipen->Entries[keyword_id][entrynr] = upNewEntry (entry,count);
	pUnipen->entry_sequence[count] = pUnipen->Entries[keyword_id][entrynr];

	*new_entrynr = entrynr;
	return keyword_id;
}

upBool upReadNextFile (tUPUnipen *pUnipen, char *fname, int *n, int got_version)
{
	char *stream;
	char *entry,*try_scan;
	char includefile[512];
	int keyword_id,entrynr,nsamples,first_sample=0;
	FILE *fp;
	tUPEntry ***entries=pUnipen->Entries;
	int stream_offset,nbytes;

	if ((fp=upOpenIncludeFile(pUnipen,fname,"rb",&nbytes))==NULL) {
		fprintf (stderr,"unable to open %s for input!!\n",fname);
		return 0;
	}
	if ((stream=(char *)calloc(nbytes+3,sizeof(char)))==NULL) {
		fprintf (stderr,"unable to allocate %d bytes for reading file %s!!\n",nbytes,fname);
		return 0;
	}
	if (fread(stream,sizeof(char),nbytes,fp)!=nbytes) {
		fprintf (stderr,"read error, unable to scan all %d bytes from %s\n!",nbytes,fname);
		return 0;
	}
	fclose(fp);
	stream_offset = 0;
	pUnipen->nunipens++;
	if (pUnipen->nunipens==1) {
		up_alloc(pUnipen->file_contents,char *,1);
	} else {
		up_realloc(pUnipen->file_contents,char *,pUnipen->nunipens);
	} pUnipen->file_contents[pUnipen->nunipens-1] = stream;

	while ((entry=NextEntry(stream+stream_offset,&stream_offset))!=NULL) {

		keyword_id = upAdministrateEntry (pUnipen,entry,&entrynr,*n);
		if (keyword_id<0) {
			if (!got_version) {
				fprintf (stderr,"file '%s' is no UNIPEN file!\n",fname);
				fprintf (stderr,"this is decided because the first entry is no '.VERSION 1.0'\n");
				fprintf (stderr,"and because the first entry is no '.INCLUDE'\n");
				fprintf (stderr,"the entry was [%s]\n",entry);
				free(stream);
				return 0;
			}
			continue;
		}
		(*n)++; /* increment the count of this entry */

		if (!got_version) {
			if (!is_a_version_entry(entry)) {
				if (keyword_id==pUnipen->IncludeId) {
					if (sscanf(entry,".INCLUDE%s",includefile)==1) {
						(void) upReadNextFile (pUnipen,includefile,n,0);
						got_version = 1;
						continue;
					} else {
						fprintf (stderr,".INCLUDE requires a file argument!\n");
						free(stream);
						return 0;
					}
				} else {
					fprintf (stderr,"file '%s' is no UNIPEN file!\n",fname);
					fprintf (stderr,"this is decided because the first entry is no '.VERSION 1.0'\n");
					fprintf (stderr,"and because the first entry is no '.INCLUDE'\n");
					fprintf (stderr,"the entry was [%s]\n",entry);
					free(stream);
					return 0;
				}
			}
			got_version = 1;
		}

		if (keyword_id == pUnipen->PenLapseId   ||
				keyword_id    == pUnipen->PenDownId ||
				keyword_id    == pUnipen->PenUpId) { /* is it a PENSTREAM?? */
/*			if (keyword_id==pUnipen->PenLapseId)
				continue; */
			try_scan = upNthString(entry,1);    /* does it contain samples ??? */
			if (try_scan!=NULL) {
				if (try_scan[0]!='\0') {
					if (strchr("-+0123456789",try_scan[0])!=NULL) {
						pUnipen->NrOfPenStreams++;
						nsamples = upNSamples(entry);
						pUnipen->stream_sequence = (tUPStream *) upChangeMemory
							(pUnipen->stream_sequence, (pUnipen->NrOfPenStreams)*sizeof (tUPStream));
						pUnipen->stream_sequence[pUnipen->NrOfPenStreams-1].entry = entries[keyword_id][entrynr];
						pUnipen->stream_sequence[pUnipen->NrOfPenStreams-1].first_sample = first_sample;
						pUnipen->stream_sequence[pUnipen->NrOfPenStreams-1].nsamples = nsamples;
						first_sample += nsamples;
						if (keyword_id!=pUnipen->PenLapseId) {
							pUnipen->valid_streams = (int *) upChangeMemory (pUnipen->valid_streams,(pUnipen->nvalid_streams+1)*sizeof(int));
							pUnipen->valid_streams[pUnipen->nvalid_streams++] = pUnipen->NrOfPenStreams-1;
						}
					}
				}
			}
		} else if (keyword_id==pUnipen->IncludeId) {
			if (sscanf(entry,".INCLUDE%s",includefile)==1) {
				(void) upReadNextFile (pUnipen,includefile,n,1);
			} else {
				fprintf (stderr,".INCLUDE requires a file argument!\n");
				free(stream);
				return 0;
			}
		}
	}
	pUnipen->TotalNrOfEntries = *n;
	return 1;
}

void upOverrideHierarchy (tUPUnipen *pUnipen)
{
	tUPEntry *src_entry, *dst_entry;
	int nhier;

	nhier = mNrOfHierarchies(pUnipen);
	dst_entry = pUnipen->Entries[pUnipen->HierarchyId][0];
	src_entry = pUnipen->Entries[pUnipen->HierarchyId][nhier-1];
	fprintf (stderr,"overriding [%s] with [%s]\n"
		,dst_entry->Entry,src_entry->Entry);
	pUnipen->Entries[pUnipen->HierarchyId][0] = src_entry;
	pUnipen->Entries[pUnipen->HierarchyId][nhier-1] = dst_entry;
}

upBool upNextFile (tUPUnipen *pUnipen, char *fname)
{
	tUPEntry ***entries=pUnipen->Entries;
	int i,j;
	char *entry,level[256];
	char *path,*pptr;

	if (pUnipen->nunipens!=0) {
		up_free(pUnipen->cur_file_open);
		for (i=0;i<pUnipen->nunipens;i++)
			free(pUnipen->file_contents[i]);
		up_free(pUnipen->file_contents);
		pUnipen->nunipens = 0;
	}
	/* delete previous entries */
	for (i=0;i<pUnipen->NrOfKeywords;i++) {
		for (j=0;j<pUnipen->NrOfEntries[i];j++)
			up_free(entries[i][j]);
		pUnipen->NrOfEntries[i] =  0;
        pUnipen->CurrOffsetEntries[i] = -1;
	}
	/* free all pointers and reset all counts */
	for (i=0;i<pUnipen->NrOfLevels;i++)
		up_free(pUnipen->Levels[i]);
	up_free(pUnipen->Levels);
	up_free(pUnipen->NrOfSegmentsInLevel);
	for (i=0;i<pUnipen->NrOfCoords;i++)
		up_free(pUnipen->Coords[i]);
	up_free(pUnipen->Coords);
	pUnipen->NrOfLevels          = 0;
	pUnipen->NrOfCoords          = 0;
	pUnipen->TotalNrOfEntries    = 0;
	pUnipen->last_pen_stream     = -1;
	pUnipen->nvalid_streams      = 0;
	pUnipen->NrOfPenStreams      = 0;
	mBoundedDelineation(pUnipen) = mUnknownDelineation(pUnipen) = 0;
	up_free(pUnipen->entry_sequence);
	up_free(pUnipen->stream_sequence);
	up_free(pUnipen->valid_streams);
	j = 0;
	/* free and setup search path */
	up_free(pUnipen->cur_path);
	up_alloc(pUnipen->cur_path,char,strlen(fname)+1);
	path = pUnipen->cur_path;
	sprintf (path,"%s",fname);
	while ((pptr=strchr(path,'/'))!=NULL)
		path = pptr+1;
	if (path!=pUnipen->cur_path)
		*(path-1) = '\0';
	pUnipen->cur_file_open = upNewString(fname);
	if (upReadNextFile (pUnipen,pUnipen->cur_file_open,&j,0)) {
		/* hierarchy */
		pUnipen->NrOfLevels = 0;
		pUnipen->Levels     = NULL;
		if (mNrOfSets        (pUnipen) >  1) {
			fprintf (stderr,errMultipleSets       , pUnipen->cur_file_open, keySet);
			return 0;
		}
		if (mNrOfHierarchies (pUnipen) >  1) {
			fprintf (stderr,errMultipleHierarchies, pUnipen->cur_file_open, keyHierarchy);
			upOverrideHierarchy(pUnipen);
		}
		if (mNrOfHierarchies (pUnipen) == 0) {
			fprintf (stderr,errNoHierarchy        , pUnipen->cur_file_open, keyHierarchy);
			return 0;
		}

		/* levels in hierarchy */
		entry = entries[pUnipen->HierarchyId][0]->Entry;
		entry = upNthString (entry,1);
		while (entry!=NULL) {
			/* increase nr of levels */
			pUnipen->NrOfLevels++;
			/* reallocate levels */
			pUnipen->Levels = (char **) upChangeMemory (pUnipen->Levels, pUnipen->NrOfLevels * sizeof (char *));
			/* copy level */
			sscanf(entry,"%s",level);
			pUnipen->Levels [pUnipen->NrOfLevels-1] = upNewString (level);
			entry = upNthString (entry,1);
		}
		if (pUnipen->NrOfLevels == 0) {
			fprintf (stderr,errNoHierarchy, pUnipen->cur_file_open, keyHierarchy);
			return 0;
		}

		/* segments in levels */
		up_alloc(pUnipen->NrOfSegmentsInLevel,int,pUnipen->NrOfLevels);
		for (i=0;i<mNrOfSegments(pUnipen);i++) {
			sscanf(upNthString(entries[pUnipen->SegmentId][i]->Entry,1),"%s",level);
			for (j=pUnipen->NrOfLevels-1;j>=0;j--) {
				if (strcmp(level,pUnipen->Levels[j])==0) {
					pUnipen->NrOfSegmentsInLevel[j]++;
					break;
				}
			}
		}

		/* coord */
		pUnipen->NrOfCoords = 0;
		pUnipen->Coords     = NULL;
		if (mNrOfCoords (pUnipen) >  1) {
			fprintf (stderr,errMultipleCoords, pUnipen->cur_file_open, keyCoord);
			return 0;
		}
		if (mNrOfCoords (pUnipen) == 0) {
			fprintf (stderr,errNoCoord       , pUnipen->cur_file_open, keyCoord);
			return 0;
		}
		entry = entries[pUnipen->CoordId][0]->Entry;
		entry = upNthString (entry,1);
		while (entry!=NULL) {
			/* increase nr of coordinates */
			pUnipen->NrOfCoords++;
			/* reallocate coordinates */
			pUnipen->Coords = (char **) upChangeMemory (pUnipen->Coords, pUnipen->NrOfCoords * sizeof (char *));
			/* copy coordinates */
			sscanf(entry,"%s",level);
			pUnipen->Coords [pUnipen->NrOfCoords-1] = upNewString (level);
			entry = upNthString (entry,1);
		}
		if (pUnipen->NrOfCoords < 2) {
			fprintf (stderr,"FATAL: file %s should at least contain .COORD X Y! (was %s)\n", pUnipen->cur_file_open,entry);
			return 0;
		}
		if (strcmp(pUnipen->Coords[0],"X")!=0) {
			fprintf (stderr,"FATAL: file %s should at least contain .COORD X Y! (was %s)\n", pUnipen->cur_file_open,entry);
			return 0;
		}
		if (strcmp(pUnipen->Coords[1],"Y")!=0) {
			fprintf (stderr,"FATAL: file %s should at least contain .COORD X Y! (was %s)\n", pUnipen->cur_file_open,entry);
			return 0;
		}
		if (pUnipen->Entries[pUnipen->SampleRateId]==NULL) {
			/* if COORDS contains T, then  still ok */
			for (i=0;i<pUnipen->NrOfCoords;i++) {
				if (strcmp("T",pUnipen->Coords[i])==0)
					break;
			}
			if (i==pUnipen->NrOfCoords) {
				fprintf (stderr,"UNIPEN file has no .POINTS_PER_SECOND and COORDS has no 'T'!!\n");
				fprintf (stderr,"taking default samplerate=100Hz\n");
			}
		}
		/* do we have T,P,B,RHO,THETA or PHI coordinates? */
		pUnipen->has_z = -1;
		pUnipen->has_p = -1;
		pUnipen->has_t = -1;
		pUnipen->has_b = -1;
		pUnipen->has_rho = -1;
		pUnipen->has_theta = -1;
		pUnipen->has_phi = -1;
		for (i=0;i<pUnipen->NrOfCoords;i++) {
			if (strcmp("Z",pUnipen->Coords[i])==0) {
				if (pUnipen->has_z!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'Z'!\n",entry);
					exit(1);
				}
				pUnipen->has_z = i;
			}
			if (strcmp("P",pUnipen->Coords[i])==0) {
				if (pUnipen->has_p!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'P'!\n",entry);
					exit(1);
				}
				pUnipen->has_p = i;
				fprintf (stderr,"file contains .COORD P\n");
			}
			if (strcmp("B",pUnipen->Coords[i])==0) {
				if (pUnipen->has_b!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'B'!\n",entry);
					exit(1);
				}
				pUnipen->has_b = i;
				fprintf (stderr,"file contains .COORD B, this will NOT be used!\n");
			}
			if (strcmp("RHO",pUnipen->Coords[i])==0) {
				if (pUnipen->has_rho!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'RHO'!\n",entry);
					exit(1);
				}
				pUnipen->has_rho = i;
				fprintf (stderr,"file contains .COORD RHO, this will NOT be used!\n");
			}
			if (strcmp("THETA",pUnipen->Coords[i])==0) {
				if (pUnipen->has_theta!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'THETA'!\n",entry);
					exit(1);
				}
				pUnipen->has_theta = i;
				fprintf (stderr,"file contains .COORD THETA, this will NOT be used!\n");
			}
			if (strcmp("PHI",pUnipen->Coords[i])==0) {
				if (pUnipen->has_phi!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'PHI'!\n",entry);
					exit(1);
				}
				pUnipen->has_phi = i;
				fprintf (stderr,"file contains .COORD PHI, this will NOT be used!\n");
			}
			if (strcmp("T",pUnipen->Coords[i])==0) {
				if (pUnipen->has_t!=-1) {
					fprintf (stderr,"entry '%s' contains multiple 'T'!\n",entry);
					exit(1);
				}
				pUnipen->has_t = i;
				fprintf (stderr,"file contains .COORD T, this will NOT be used\n");
			}
		}
		if (!pUnipen->has_z && pUnipen->has_p)
			fprintf (stderr,"Using .COORD P as z-coordinate\n");
		return 1;
	}
	else
		return 0;
}

void upDoneFile (tUPUnipen *pUnipen)
{
	int KeywordId, EntryNr, LevelNr, CoordNr;


	/* panic */
	if (pUnipen == NULL) {
		fprintf (stderr,"PANIC: upDoneFile\n");
		return;
	}


	/* keywords (nothing to do) */


	/* short cuts (nothing to do) */

	/* entries */
	for (KeywordId=0;KeywordId<pUnipen->NrOfKeywords;KeywordId=KeywordId+1) {
		for (EntryNr=0;EntryNr<pUnipen->NrOfEntries[KeywordId];EntryNr=EntryNr+1)
		{
			free(pUnipen->Entries [KeywordId][EntryNr]);
		}

		pUnipen->NrOfEntries       [KeywordId] =  0;
		pUnipen->CurrOffsetEntries [KeywordId] = -1;
		pUnipen->Entries           [KeywordId] =  (tUPEntry **) upDelMemory (pUnipen->Entries       [KeywordId]);
	}
	pUnipen->entry_sequence    = (tUPEntry **) upDelMemory (pUnipen->entry_sequence    );
	pUnipen->stream_sequence   = (tUPStream *) upDelMemory (pUnipen->stream_sequence    );
	pUnipen->valid_streams     = (int *) upDelMemory (pUnipen->valid_streams);
	pUnipen->last_pen_stream   = -1;
	pUnipen->nvalid_streams    = 0;
	pUnipen->NrOfPenStreams    = 0;
	pUnipen->TotalNrOfEntries  = 0;

	/* do *NOT* free ->NrOfEntries, CurrOffsetEntries, PrevOffsetEntries, MarkedEntries, Entries!! */

	/* hierarchy */
	for (LevelNr = 0; LevelNr < pUnipen->NrOfLevels; LevelNr = LevelNr + 1)
	{
		pUnipen->Levels [LevelNr] = upDelString (pUnipen->Levels [LevelNr]);
	}
	pUnipen->NrOfLevels = 0;
	pUnipen->Levels     = (char **) upDelMemory (pUnipen->Levels);
	pUnipen->NrOfSegmentsInLevel     = (int *) upDelMemory (pUnipen->NrOfSegmentsInLevel);
	/* coord */
	for (CoordNr = 0; CoordNr < pUnipen->NrOfCoords; CoordNr = CoordNr + 1)
	{
		pUnipen->Coords [CoordNr] = upDelString (pUnipen->Coords [CoordNr]);
	}
	pUnipen->NrOfCoords = 0;
	pUnipen->Coords     = (char **) upDelMemory (pUnipen->Coords);
}

int upGetDoubleFromEntries (tUPUnipen *pUnipen, char keyword[], double *result, int idx)
{
   char *p, *number;
   double ret;
   int id;

   if( pUnipen  == NULL) {
          fprintf(stderr,"UNIPEN not initialized yet\n");
			 *result = -9999.;
          return(0);
   } 

   if( pUnipen->Entries == NULL) {
          fprintf(stderr,"UNIPEN keywords not mapped yet (found zippo keywords)\n");
			 *result = -9999.;
          return(0);
   } 
  
   id = upGetKeywordId (pUnipen, keyword);
    
   if( pUnipen->Entries[id] == NULL) {
/*          fprintf(stderr,"UNIPEN keyword %s not mapped (yet?)\n", *          keyword);*/
			 *result = -9999.;
          return(0);
   } 

	if (pUnipen->NrOfEntries[id]<=idx) {
          fprintf(stderr,"requesting %dth keyword %s, but only %d available!\n",idx,keyword,pUnipen->NrOfEntries[id]);
			 *result = -9999.;
          return(0);
	}

   p = pUnipen->Entries [id][idx]->Entry;

   if(p == NULL) {
          fprintf(stderr,"UNIPEN keyword %s is missing\n", keyword);
			 *result = -9999.;
          return(0);
   }
   
   number = strpbrk(p,cScheiders);
   if(number == NULL) {
   	  fprintf(stderr,"UNIPEN keyword %s <R>: argument missing!\n", keyword);
			 *result = -9999.;
          return(0);
   }
   
   ret = strtod(number, &p);
   
   if(p == 0) {
   	  fprintf(stderr,"UNIPEN keyword %s: <R> argument missing!!\n", keyword);
			 *result = -9999.;
          return(0);

   }	                
	*result = ret;
   return(1);
   
}

/* a module for checking whether all labels belonging to a segment are in a file
   containing the dictionary:
	   setp 0: construct all .SEGMENT labels and sort them
		step 1: read the dictionary and sort it
		step 2: walk through the sorted arrays
*/

static int entry_compar (const void *p1, const void *p2)
{
	return strcmp ( (*((SegmentEntry **)p1))->label, (*((SegmentEntry **)p2))->label );
}

static int dictionary_compar (const void *p1, const void *p2)
{
	return strcmp (*((char **)p1), (*((char **)p2)) );
}

SegmentEntry **construct_segment_entries (tUPUnipen *pUnipen, char *level
	, int *n, int do_sort)
{
	int i;
	SegmentEntry **result;
	tUPEntry *entry;
	int lev,nsegments,cur_segnr;

	lev = upCurLevel (pUnipen,level);
	nsegments = pUnipen->NrOfSegmentsInLevel[lev];
	cur_segnr = mCurrOffsetSegment(pUnipen);

	up_alloc(result,SegmentEntry *,nsegments);
	mCurrOffsetSegment(pUnipen) = -1; /* reset the segment numbering */
	i= 0;
	while ((entry=upNextSegment(pUnipen,level,1))!=NULL) {
		if (i<nsegments) {
			up_alloc(result[i],SegmentEntry,1);
			result[i]->entry = entry;
			result[i]->label = upEntryName(entry);
		}
		i++;
	}
	if (i!=nsegments) {
		fprintf (stderr,"FATAL: expected %d %s segments, found %d\n"
			,nsegments,level,i);
		for (i=0;i<nsegments;i++) {
			free(result[i]->label);
			free(result[i]);
		}
		free(result);
		*n = 0;
		result = NULL;
	} else {
		if (do_sort)
			qsort (result,nsegments,sizeof(SegmentEntry *),entry_compar);
		*n = nsegments;
	}
	mCurrOffsetSegment(pUnipen) = cur_segnr; /* reset the segment numbering */
	return result;
}

static void delete_segment_entries (SegmentEntry **entries, int n)
{
	int i;

	for (i=0;i<n;i++) {
		free (entries[i]->label);
		free(entries[i]);
	}
	free(entries);
}

static char **construct_dictionary (char *fname, int *n)
{
	char wrd[1024];
	int i=0;
	FILE *fp;
	char **result;

	if ((fp=fopen(fname,"r"))==NULL) {
		fprintf (stderr,"unable to open dictionary file %s\n",fname);
		*n = -1;
		return NULL;
	}
	up_alloc(result,char *,1);
	while (fgets(wrd,1024,fp)!=NULL) {
		wrd[strlen(wrd)-1]='\0';
		up_alloc(result[i],char,strlen(wrd)+1);
		strcpy(result[i++],wrd);
		up_realloc(result,char *,i+1);
	}
	fclose(fp);
	qsort (result,i,sizeof(char *),dictionary_compar);
	*n = i;
	return result;
}

static void delete_dictionary (char **dictionary, int n)
{
	int i;

	for (i=0;i<n;i++)
		free(dictionary[i]);
	free(dictionary);
}

UnipenDictionary *upGetDictionary (tUPUnipen *pUnipen)
{
	int i;
	int lev = upCurLevel (pUnipen,"WORD");
	int nsegments = pUnipen->NrOfSegmentsInLevel[lev];
	UnipenDictionary *result;
	tUPEntry *entry;

	int cur_segnr = mCurrOffsetSegment(pUnipen);
	
	up_alloc(result,UnipenDictionary,1);
	up_alloc(result->words,char *,nsegments);
	result->nwords = nsegments;
	mCurrOffsetSegment(pUnipen) = -1; /* reset the segment numbering */
	i= 0;
	while ((entry=upNextSegment(pUnipen,"WORD",1))!=NULL) {
		if (i<nsegments) {
			result->words[i] = upEntryName(entry);
		}
		i++;
	}
	if (i!=nsegments) {
		fprintf (stderr,"FATAL: expected %d %s segments, found %d\n"
			,nsegments,"WORD",i);
		mCurrOffsetSegment(pUnipen) = cur_segnr; /* reset the segment numbering */
		return NULL;
	}

	mCurrOffsetSegment(pUnipen) = cur_segnr; /* reset the segment numbering */
	
	return result;
}

int upCheckDictionary (tUPUnipen *pUnipen, char *fname, char *level)
{
	int i,j,last_found,found,comp,ndict,nsegments;
	char *query,**dictionary;
	SegmentEntry **segment_entries;

	int ret_code = 1;

	if ((dictionary=construct_dictionary(fname,&ndict))==NULL) 
		return 0;
	if ((segment_entries=
			construct_segment_entries(pUnipen,level,&nsegments,1))==NULL) 
		return 0;
	last_found = 0;
	for (i=0;i<nsegments;i++) {
		j = last_found;
		found = 0;
		query = segment_entries[i]->label;
		while (j<ndict && !found) {
			comp = strcmp(dictionary[j],query);
			if (comp==0) {
				found = 1;
				last_found = j;
			} else if (comp<0) {
				j++;
			} else {
				break;
			}
		}
		if (!found) {
			ret_code = 0;
			fprintf (stderr,"entry %s not in dictionary `%s'!!\n"
				,segment_entries[i]->entry->Entry
				,fname);
			fprintf (stderr,"    entrynr was %d query was `%s'!!\n"
				,i,query);
		}
	}
	delete_dictionary (dictionary,ndict);
	delete_segment_entries (segment_entries,nsegments);
	return ret_code;
}

int upGetVerbosity(tUPUnipen *pUnipen)
{
        return(logVerbosity(pUnipen));
}

void upSetVerbosity(tUPUnipen *pUnipen, int log)
{       
        logVerbosity(pUnipen) = log;
}
