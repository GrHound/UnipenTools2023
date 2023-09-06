#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include <uplib.h>

#include "writer_identifications.h"

/* writer_identifications.c
   - Reads a 'labeled writer-code file', containing entries
     'number writer-name dataset path', like, e.g.,
00241                 teun    plucoll teun/set3.dat
00292              mariska    papyrus NIC-papyrus-mariska.dat
21284              wcdr451      nist7 8/bbd/wcdr/wcdr451.dat
*/

#define NDATASETS   3
#define PLUCOLL_SET 0
#define PAPYRUS_SET 1
#define NIST7_SET   2

typedef struct {
	char *name;
	char *path;
} DataSet;

static DataSet datasets[NDATASETS] = {
	{ "plucoll", "/fxt/projects/unipen/plucoll" },
	{ "papyrus", "/ext/projects/nici-unipen/unipen_files" },
	{ "nist7"  , "/fxt/users/vuurpijl/train_r01_v07/data" },
};

int dataset (char *dset)
{
	int i;

	for (i=0;i<NDATASETS;i++)
		if (strcmp(datasets[i].name,dset)==0) {
			return i;
		}
	fprintf (stderr,"dataset '%s' unknown!!\n",dset);
	return -1;
}

static WriterIdentification *writer_identifications = NULL;
static int nwriter_ids = 0;

char *construct_path (char *fname)
{
	static char path[512];
	static char here[512];

	getcwd(here,512);
	/* if here is part of fname, we know fname is an absolute path */
	if (strstr(fname,here)!=NULL)
		return fname;
	sprintf (path,"%s/%s",here,fname);
	return path;
}

void updirectFname_to_writer_id(char *fname, char *codestr) 
{
	int i;
	
	strcpy(codestr,basename(fname));
	i = strlen(codestr) - 1;
	while(i > 0) {
		if(codestr[i] == '.') {
			codestr[i] = 0;
			return;
		}
		--i;
	}
}

static int writer_code (char *fname)
{
	int i;
	char *the_path;

	the_path = construct_path(fname);
	
	for (i=0;i<nwriter_ids;i++) {
		if (strstr(the_path,writer_identifications[i].path)!=NULL)
				return i;
	}
	fprintf (stderr,"file [%s] unknown (path constructed was '%s')!!\n"
		,fname,the_path);
	return -1;
}

void insert_writer_identification (int idx, char *wname, char *dset, char *path)
{
	int i;

	if (nwriter_ids==idx) {
		if (nwriter_ids==0) {
			writer_identifications = (WriterIdentification *) malloc (sizeof(WriterIdentification));
		}
		else {
			writer_identifications = (WriterIdentification *) realloc (writer_identifications,
				(nwriter_ids+1)*sizeof(WriterIdentification));
		}
	} else if (nwriter_ids<idx) {
		fprintf (stderr,"strange.... requesting to insert %d in array [0-%d]!\n"
			,idx,nwriter_ids);
		if (nwriter_ids==0) {
			writer_identifications = (WriterIdentification *) malloc ((idx+1)*sizeof(WriterIdentification));
		}
		else {
			writer_identifications = (WriterIdentification *) realloc (writer_identifications,
				(idx+1)*sizeof(WriterIdentification));
		}
		for (i=nwriter_ids;i<idx;i++)
			writer_identifications[i].idx = -1;
		nwriter_ids = idx;
	} else {
		if (writer_identifications[idx].idx != -1) {
			fprintf (stderr,"request to fill %s at %d, but it already contains %s!\n"
				,wname,idx,writer_identifications[idx].writer_name);
			return;
		}
	}
	writer_identifications[nwriter_ids].idx         = idx;
	writer_identifications[nwriter_ids].writer_name = strdup(wname);
	writer_identifications[nwriter_ids].dataset     = dataset(dset);
	writer_identifications[nwriter_ids].path        = strdup(path);
	nwriter_ids                                    += 1;
}

int  read_writer_identifications (char *fname)
{
	FILE *fp;
	int n,idx;
	char wname[64],dset[64],path[256];

	if ((fp=fopen(fname,"r"))==NULL) {
		fprintf (stderr,"unable to open writer_identifications '%s'!!\n",fname);
		exit(1);
	}
	n = 0;
	while (fscanf(fp,"%d%s%s%s",&idx,wname,dset,path)==4) {
		insert_writer_identification(idx,wname,dset,path);
		n++;
	}
	fclose(fp);
	return n;
}

int find_index (tUPEntry *entry, char *name_requested, tUPEntry **entries, int nentries)
{
	int i,idx;
	char *nxt_name,nxt_hierarchy[64],hierarchy[64];

	if (sscanf(entry->Entry,".SEGMENT %s",hierarchy)!=1) {
		fprintf (stderr,"unable to scan hierarchy from entry [%s]\n!!"
			,entry->Entry);
		return -1;
	}

	idx = 0;
	for (i=0;i<nentries;i++) {
		if (sscanf(entries[i]->Entry,".SEGMENT %s",nxt_hierarchy)!=1) {
			fprintf (stderr,"unable to scan hierarchy from entry %d=[%s]\n!!"
				,i,entries[i]->Entry);
			return -1;
		}
		if (strcmp(hierarchy,nxt_hierarchy)==0) {
			if (strcmp(entry->Entry,entries[i]->Entry)==0)
				return idx;
			else {
				nxt_name = upEntryName (entries[i]);
				if (strcmp(name_requested,nxt_name)==0)
					idx++;
				free(nxt_name);
			}
		}
	}
	return -1;
}

int updirFilename2WriterCode (char *fname)
{
	return writer_code(fname);
}

char *updirEntry2WriterCode (tUPEntry *entry, tUPEntry **entries
	, int nentries, int wcode)
{
	static char result[256];
	char hierarchy[64],*name;
	int idx;

	if (sscanf(entry->Entry,".SEGMENT %s %s %s \"%s\""
			,hierarchy,result,result,result)!=4) {
		fprintf (stderr,"unable to scan hierarchy from entry [%s]\n!!"
			,entry->Entry);
	}
	name = upEntryName(entry);
	idx   = find_index (entry,name,entries,nentries);
	if (wcode<0) {
		sprintf (result,"%c/\?\?\?\?\?/%c/%d"
			,name[0]
			,hierarchy[0]
			,idx);
	} else {
		sprintf (result,"%c/%05d/%c/%d"
			,name[0]
			,wcode
			,hierarchy[0]
			,idx);
	}
	free(name);
	return result;
}

void parse_writercode (char *wcode, WriterIdentification *writer_id)
{
	char *wsrc,*ptr;
	int idx;

	wsrc = wcode;
	ptr = strchr(wcode,'/');
	ptr[0] = '\0';
	strcpy(writer_id->seg_name,wcode);

	wcode = ptr+1;
	ptr = strchr(wcode,'/');
	ptr[0] = '\0';
	if (sscanf(wcode,"%d",&idx)!=1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wsrc);
		exit(1);
	}
	if (idx==-1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wsrc);
		exit(1);
	}
	writer_id->idx         = idx;
	writer_id->writer_name = writer_identifications[idx].writer_name;
	writer_id->dataset     = writer_identifications[idx].dataset;
	writer_id->path        = writer_identifications[idx].path;

	wcode = ptr+1;
	switch (wcode[0]) {
		case 'C':
			strcpy(writer_id->level,"CHARACTER");
			break;
		case 'W':
			strcpy(writer_id->level,"WORD");
			break;
		default:
			fprintf (stderr,"writer_code '%s' contains unknown hierarchy '%c'!!\n"
				,wsrc,wcode[0]);
			exit(1);
	}
	idx = -1;
	if (sscanf(wcode+2,"%d",&idx)!=1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wcode);
		exit(1);
	}
	if (idx==-1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wcode);
		exit(1);
	}
	writer_id->seg_idx = idx;
	fprintf (stderr,"%d=%s lev=%s idx=%d label=%s [%s] %s\n"
		,writer_id->idx
		,writer_id->writer_name
		,writer_id->level
		,writer_id->seg_idx
		,writer_id->seg_name
		,datasets[writer_id->dataset].name
		,writer_id->path);
}

char *writerid_2_upfile (WriterIdentification *writer_id)
{
	static char result[512];

	sprintf (result,"%s/%s"
		,datasets[writer_id->dataset].path
		,writer_id->path);
	return result;
}


#ifdef _TEST_W_
int main (int argc, char *argv[])
{
	char line[300];
	int i = 0;

	read_writer_identifications(WRITER_ID_FILE);
	while (fgets(line,300,stdin)!=NULL)
		printf ("%s\n",updirEntry2WriterCode(line,argv[1],i++));
	return 0;
}
#endif
