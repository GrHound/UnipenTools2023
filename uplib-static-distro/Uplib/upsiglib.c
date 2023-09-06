
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uplib.h"
#include "upsiglib.h"

/*****************************/
/* how to parse delineations */

static char *next_component (char *d, char *c)
{
	char *result;
	int i,found=0,n=0;

	for (i=0;i<strlen(d)&&!found;i++)
		switch (d[i]) {
			case ',': case ' ': case '\t':
				found = 1;
				*c = d[i];
				break;
			default:
				n++;
				break;
		}
	if (i==0) {
		*c = ' ';
		return NULL;
	}
	if (i==strlen(d)) {
		*c = ' ';
	}
	up_alloc(result,char,n+1);
	for (i=0;i<n;i++)
		result[i] = d[i];
	return result;
}

sigDelineation *sigCopyDelineation (sigDelineation *src)
{
	sigDelineation *del;
	int i;

	up_alloc(del,sigDelineation,1);
	del->ndels = src->ndels;
	up_alloc(del->delineations,sigSampleDelineation,del->ndels);
	for (i=0;i<del->ndels;i++) {
		del->delineations[i].first_streamnr = src->delineations[i].first_streamnr;
		del->delineations[i].last_streamnr  = src->delineations[i].last_streamnr;
		del->delineations[i].first_samplenr = src->delineations[i].first_samplenr;
		del->delineations[i].last_samplenr  = src->delineations[i].last_samplenr;
	}
	return del;
}

sigDelineation *sigCreateBoundingDelineation (sigDelineation *src)
{
	sigDelineation *del;

	up_alloc(del,sigDelineation,1);
	del->ndels = 1;
	up_alloc(del->delineations,sigSampleDelineation,1);
	del->delineations[0].first_streamnr = src->delineations[0].first_streamnr;
	del->delineations[0].first_samplenr = src->delineations[0].first_samplenr;
	del->delineations[0].last_streamnr  = src->delineations[src->ndels-1].last_streamnr;
	del->delineations[0].last_samplenr  = src->delineations[src->ndels-1].last_samplenr;
	return del;
}


void sigPrintCharStreams (sigCharStream *s)
{
	int i;

fprintf (stderr,"%d charstreams\n",s->nstreams);
	for (i=0;i<s->nstreams;i++) {
fprintf (stderr,"   charstream[%d] (%d samples)\n",i,s->nsamples[i]);
/*		fprintf (stderr,"   [%s]\n",s->streams[i]); */
	}
}

void sigOutputDelineation (FILE *fp, sigDelineation *del)
{
	int i;

	if (del->ndels<=0) return;
	fprintf (stderr,"%d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[0].last_streamnr
		,del->delineations[0].last_samplenr);
	for (i=1;i<del->ndels;i++) {
		fprintf (stderr,",%d:%d-%d:%d"
			,del->delineations[i].first_streamnr
			,del->delineations[i].first_samplenr
			,del->delineations[i].last_streamnr
			,del->delineations[i].last_samplenr);
	}
}

void sigPrintDelineation (char *msg, sigDelineation *del)
{
	fprintf (stderr,"%s: %d delineations\n",msg,del->ndels);
	sigOutputDelineation(stderr,del);
	fprintf (stderr,"\n");
}

sigDelineation *sigParseDelineation (char *delineation)
{
	char *component,*ptr=delineation;
	char scheider;
	sigDelineation *del = NULL;
	int n = 0,N0,M0,N1,M1;
	
	component = next_component(ptr,&scheider);
	while (component!=NULL) {
		if (n==0) {
			up_alloc(del,sigDelineation,1);
			up_alloc(del->delineations,sigSampleDelineation,1);
		}
		else {
			up_realloc(del->delineations,sigSampleDelineation,n+1);
		}
		ptr += strlen(component)+1;
		N0 = M0 = N1 = M1 = -1;
		/* component: N0 or N0-N1 or N0:M0-N1 or N0-N1:M1 or N0:M0-N1:M1 */
		if (sscanf(component,"%d:%d-%d:%d",&N0,&M0,&N1,&M1)!=4) {
		if (sscanf(component,"%d-%d:%d",&N0,&N1,&M1)!=3) {
		if (sscanf(component,"%d:%d-%d",&N0,&M0,&N1)!=3) {
		if (sscanf(component,"%d-%d",&N0,&N1)!=2) {
			if (sscanf(component,"%d",&N0)!=1)
				return NULL;
			else
				N1 = N0;
		}}}}
		del->delineations[n].first_streamnr = N0;
		del->delineations[n].last_streamnr =  N1;
		del->delineations[n].first_samplenr = M0;
		del->delineations[n].last_samplenr =  M1;
		free(component);
		if (scheider==' ')
			component=NULL;
		else
			component = next_component(ptr,&scheider);
		n++;
		del->ndels = n;
	}
	return del;
}

int delineation_contains (sigSampleDelineation *del, int str, int sam)
{
	if (del->first_streamnr==del->last_streamnr && del->first_streamnr==str)
		if ( (del->first_samplenr<= del->first_samplenr + sam) && (del->last_samplenr>=del->first_samplenr+sam-1) ) {
			return 1;
		}

	if ((del->first_streamnr==str && del->first_samplenr<= del->first_samplenr + sam)|| del->first_streamnr<str)
		if ( (del->last_streamnr==str&&del->last_samplenr>=sam) || del->last_streamnr>str) {
			return 1;
		}
	return 0;
}

sigDelineation *sigBounds2Delineation (tUPUnipen *pUnipen, tUPEntry *entry, sigSignal *sig
	,int s_start, int s_end)
{
	int i,j,n,s,penlift,samplenr,streamnr;
	int str_start,str_end;
	int sam_start,sam_end;
	sigDelineation *sig_del,*del;
	sigCharStream *streams;

	if (s_start>s_end) {
		fprintf (stderr,"are you kiddin'? start %d must be <= end %d\n",s_start,s_end);
		return NULL;
	}
	if ( (s_start>=sig->nsamples) || (s_end>=sig->nsamples) ) {
		fprintf (stderr,"ERROR: delineation %d-%d must be within [0-%d]!\n"
			,s_start,s_end,sig->nsamples);
		return NULL;
	}

	if ((sig_del=sigEntry2Delineation (pUnipen,entry))==NULL)
		return NULL;
	if ((streams=sigGetSamples(pUnipen,entry))==NULL)
		return NULL;
	streamnr = sig_del->delineations[0].first_streamnr;
	penlift = (sig->z[0]>0); /* may not be good .... */
	j = sig_del->delineations[0].first_samplenr;


	/* search for the delineation containing 's_start' */
	/* let 's' always point into the parent signal */

	samplenr = 0;
	for (s=0;s<s_start;s++) {
		if (sig->z[s]!=-1) {
				samplenr++;
		}
	}
	j = samplenr;
	i = 0;
	while (j>=streams->nsamples[i]) {
		j -= streams->nsamples[i++];
	}
	str_start = i + sig_del->delineations[0].first_streamnr;
	sam_start = j;

	/* search for the delineation containing 's_end' */

	for (;s<s_end;s++) {
		if (sig->z[s]!=-1) {
				samplenr++;
		}
	}
	j = samplenr;
	i = 0;
	while (j>=streams->nsamples[i]) {
		j -= streams->nsamples[i++];
	}
	str_end = i+sig_del->delineations[0].first_streamnr;
	sam_end = j;

	sigDeleteCharStream (streams);

	/* we now we have at least one delineation */
	up_alloc(del,sigDelineation,1);
	up_alloc(del->delineations,sigSampleDelineation,1);
	n = del->ndels = 1;

	/* look for the delineation containing 'str_start:sam_start' */
	for (i=0;i<sig_del->ndels;i++) {
		if (delineation_contains(&(sig_del->delineations[i]),str_start,sam_start)) {
			del->delineations[0].first_streamnr = str_start;
			if (str_start==sig_del->delineations[i].first_streamnr)
				del->delineations[0].first_samplenr = sig_del->delineations[i].first_samplenr+sam_start;
			else
				del->delineations[0].first_samplenr = sam_start;
			if (delineation_contains(&(sig_del->delineations[i]),str_end,sam_end)) {
				del->delineations[0].last_streamnr = str_end;
				if (str_end==sig_del->delineations[i].first_streamnr)
					del->delineations[0].last_samplenr = sig_del->delineations[i].first_samplenr+sam_end;
				else
					del->delineations[0].last_samplenr = sam_end;
				sigFreeDelineation (sig_del);
				return del;
			}
			else {
				del->delineations[0].last_streamnr = sig_del->delineations[i].last_streamnr;
				del->delineations[0].last_samplenr = sig_del->delineations[i].last_samplenr;
			}
			break;
		}
	}

	/* append any delineations up to 'str_end:sam_end' */
	i++;
	for (;i<sig_del->ndels;i++) {
		up_realloc(del->delineations,sigSampleDelineation,n+1);
		del->delineations[n].first_streamnr = sig_del->delineations[i].first_streamnr;
		del->delineations[n].first_samplenr = sig_del->delineations[i].first_samplenr;
		del->delineations[n].last_streamnr  = sig_del->delineations[i].last_streamnr;
		del->delineations[n].last_samplenr  = sig_del->delineations[i].last_samplenr;
		n++;
		del->ndels = n;
		if (delineation_contains(&(sig_del->delineations[i]),str_end,sam_end)) {
				del->delineations[n-1].last_streamnr = str_end;
				if (str_end==sig_del->delineations[i].first_streamnr)
					del->delineations[n-1].last_samplenr = sig_del->delineations[i].first_samplenr+sam_end;
				else
					del->delineations[n-1].last_samplenr = sam_end;
				sigFreeDelineation (sig_del);
				return del;
		}
	}

	sigFreeDelineation (sig_del);
	return del;
}

char *sigDelineation2String (sigDelineation *del)
{
	char *delptr,*delineation;
	char delbuf[128];
	int i;

	if (del==NULL)
		return NULL;
	if (del->ndels==0)
		return NULL;
	sprintf (delbuf,"%d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[0].last_streamnr
		,del->delineations[0].last_samplenr);
	delineation = strdup(delbuf);
	for (i=1;i<del->ndels;i++) {
		sprintf (delbuf,",%d:%d-%d:%d"
			,del->delineations[i].first_streamnr
			,del->delineations[i].first_samplenr
			,del->delineations[i].last_streamnr
			,del->delineations[i].last_samplenr);
		delptr = (char *) realloc (delineation,strlen(delineation)+strlen(delbuf)+1);
		delptr[strlen(delineation)] = '\0';
		strcat (delptr,delbuf);
		delineation = delptr;
	}
	return delineation;
}

int sigNSamplesInDelineation (tUPUnipen *pUnipen, sigDelineation *del)
{
	int i,n;
	sigCharStream *streams;

	if ((streams=sigDelineation2Charstream(pUnipen,del))==NULL) {
		return 0;
	}
	n = streams->nsamples[0];
	for (i=1;i<streams->nstreams;i++)
		n += streams->nsamples[i];
	sigDeleteCharStream (streams);
	return n;
}

sigCharStream *sigDelineation2Charstream (tUPUnipen *pUnipen, sigDelineation *del)
{
	char *entry,*ptr;
	char **streams;
	int *nsamples;
	int nstreams   = 0;
	tUPStream *sequences = pUnipen->stream_sequence;
	int idx,i,j,n,mult=pUnipen->NrOfCoords;
	sigSampleDelineation *s;
	char streamtype[50];
	sigCharStream *result;
	int *valid = pUnipen->valid_streams;
	
	up_alloc(result,sigCharStream,1);
	nsamples = NULL;
	streams = NULL;

	for (i=0;i<del->ndels;i++) {

		/* set and check delineations */
		s = &(del->delineations[i]);
		if (s->first_streamnr<0) {
			fprintf (stderr,"delineation %d:%d must be within [0:%d]!\n"
				,s->first_streamnr,s->last_streamnr,pUnipen->nvalid_streams);
			free(result);
			return NULL;
		}
		if (s->last_streamnr>=pUnipen->nvalid_streams) {
			fprintf (stderr,"delineation %d:%d must be within [0:%d>!\n"
				,s->first_streamnr,s->last_streamnr,pUnipen->nvalid_streams);
			free(result);
			return NULL;
		}

		if (i==0) {
			if ((nsamples=(int *) calloc (1,sizeof(int)))==NULL) {
				fprintf (stderr,"unable to allocate memory for char streams!\n");
				free(result);
				return NULL;
			}
			if ((streams=(char **) calloc (1,sizeof(char *)))==NULL) {
				fprintf (stderr,"unable to allocate memory for char streams!\n");
				free(result);
				return NULL;
			}
		}
		else {
			if ((nsamples=(int *) realloc (nsamples,(nstreams+1)*sizeof(int)))==NULL) {
				fprintf (stderr,"unable to allocate memory for char streams!\n");
				free(result);
				return NULL;
			}
			if ((streams=(char **) realloc (streams,(nstreams+1)*sizeof(char *)))==NULL) {
				fprintf (stderr,"unable to allocate memory for char streams!\n");
				free(result);
				return NULL;
			}
		}

		j = s->first_streamnr;

		if (j==s->last_streamnr) {

			if (s->first_samplenr!=-1) {
				entry = upNthString(sequences[valid[j]].entry->Entry,s->first_samplenr*mult+1);
				if (entry==NULL) {
					fprintf (stderr,"FATAL: unable to get sample %d from stream  %d (mul=%d)!\n"
						,s->first_samplenr,s->first_streamnr,mult);
					fprintf (stderr,"was looking at valid[%d]=%d , sequence[%d] has %d samples\n",
						j,valid[j],valid[j],sequences[valid[j]].nsamples);
					free(result);
					return NULL;
				}
				if (s->last_samplenr!=-1) {
					nsamples[nstreams] = n = s->last_samplenr-s->first_samplenr+1;
					if (sscanf(sequences[valid[j]].entry->Entry,"%s",streamtype)!=1) {
						fprintf (stderr,"unable to determine streamtype for stream: %s\n",
							sequences[valid[j]].entry->Entry);
						free(result);
						return NULL;
					}
					if ((ptr=upNthString(entry,(n+1)*mult))==NULL) {
						idx = strlen(entry)+strlen(streamtype)+2;
						streams[nstreams] = (char *) upNewMemory(idx);
						sprintf (streams[nstreams++],"%s\n%s",streamtype,entry);
					}
					else {
						idx = ptr-entry+strlen(streamtype);
						streams[nstreams] = (char *) upNewMemory(idx+3);
						sprintf (streams[nstreams],"%s\n",streamtype);
						memcpy(&(streams[nstreams][strlen(streamtype)+1]),entry,ptr-entry);
						streams[nstreams++][idx] = '\0';
					}
				}
			}
			else {
				if (s->last_samplenr!=-1) { /* interesting, a delineation N-N:M, interpret it as N:0-N:M */
					entry = sequences[valid[j]].entry->Entry;
					nsamples[nstreams] = n = s->last_samplenr;
					if (sscanf(entry,"%s",streamtype)!=1) {
						fprintf (stderr,"unable to determine streamtype for stream: %s\n",entry);
						free(result);
						return NULL;
					}
					if ((ptr=upNthString(entry,(n+1)*mult+1))==NULL) {
						streams[nstreams++] = upNewString(entry);
					}
					else { /* search the end of the entry before ptr and copy */
						while (!strchr("0123456789",*(ptr-1))) ptr--;
						idx = ptr-entry;
						up_alloc(streams[nstreams],char,idx+3);
						memcpy(streams[nstreams],entry,idx);
						streams[nstreams++][idx] = '\0';
					}
				}
				else { /* a delineation N-N */
					nsamples[nstreams] = sequences[valid[j]].nsamples;
					streams[nstreams++] = upNewString(sequences[valid[j]].entry->Entry);
				}
			}
		}

		else {

			nsamples[nstreams] = n = sequences[valid[j]].nsamples;
			entry = sequences[valid[j]].entry->Entry;
			if (s->first_samplenr!=-1) {
				n -= s->first_samplenr;
				nsamples[nstreams] = n;
				sscanf(entry,"%s ",streamtype);
				if ((ptr=upNthString(entry,s->first_samplenr*mult+1))==NULL) {
					fprintf (stderr,"FATAL: unable to get sample %d from stream  %d (mul=%d)!\nentry was: %s\n"
						,s->first_samplenr,s->first_streamnr,mult,entry);
					fprintf (stderr,"was looking at valid[%d]=%d , sequence[%d] has %d samples\n",
						j,valid[j],valid[j],sequences[valid[j]].nsamples);
					free(result);
					return NULL;
				}
				up_alloc(streams[nstreams],char,strlen(streamtype)+strlen(ptr)+3);
				sprintf (streams[nstreams],"%s\n",streamtype);
				strcat(streams[nstreams++],ptr);
			}
			else
				streams[nstreams++] = upNewString(entry);
		
			if (s->last_streamnr!=-1) {
				if (s->last_samplenr!=-1) {
					for (++j;j<s->last_streamnr;j++) {
						if ((nsamples=(int *) realloc (nsamples,(nstreams+1)*sizeof(int)))==NULL) {
							fprintf (stderr,"unable to allocate memory for char streams!\n");
							free(result);
							return NULL;
						}
						if ((streams=(char **) realloc (streams,(nstreams+1)*sizeof(char *)))==NULL) {
							fprintf (stderr,"unable to allocate memory for char streams!\n");
							free(result);
							return NULL;
						}
						nsamples[nstreams] = sequences[valid[j]].nsamples;
						streams[nstreams++] = upNewString(sequences[valid[j]].entry->Entry);
					}
					if ((nsamples=(int *) realloc (nsamples,(nstreams+1)*sizeof(int)))==NULL) {
						fprintf (stderr,"unable to allocate memory for char streams!\n");
						free(result);
						return NULL;
					}
					if ((streams=(char **) realloc (streams,(nstreams+1)*sizeof(char *)))==NULL) {
						fprintf (stderr,"unable to allocate memory for char streams!\n");
						free(result);
						return NULL;
					}
					entry = sequences[valid[j]].entry->Entry;
					nsamples[nstreams] = s->last_samplenr+1;   /* LOE WAKKER WORDEN nsamples = 0-last inclusive !! */
					if ((ptr=upNthString(entry,(s->last_samplenr+1)*mult+1))==NULL) {
						streams[nstreams++] = upNewString(sequences[valid[j]].entry->Entry);
					}
					else { /* search the end of the entry before ptr and copy */
						while (!strchr("0123456789",*(ptr-1))) ptr--;
						idx = ptr-entry;
						up_alloc(streams[nstreams],char,idx+3);
						memcpy(streams[nstreams],entry,idx);
						streams[nstreams++][idx] = '\0';
					}
				}
				else {
					for (++j;j<=s->last_streamnr;j++) {
						if ((nsamples=(int *) realloc (nsamples,(nstreams+1)*sizeof(int)))==NULL) {
							fprintf (stderr,"unable to allocate memory for char streams!\n");
							free(result);
							return NULL;
						}
						if ((streams=(char **) realloc (streams,(nstreams+1)*sizeof(char *)))==NULL) {
							fprintf (stderr,"unable to allocate memory for char streams!\n");
							free(result);
							return NULL;
						}
						nsamples[nstreams] = sequences[valid[j]].nsamples;
						streams[nstreams++] = upNewString(sequences[valid[j]].entry->Entry);
					}
				}
			}
		}
	}
	result->nstreams = nstreams;
	result->nsamples = nsamples;
	result->streams = streams;
	return result;
}

sigDelineation *sigDelBounds2Delineation (tUPUnipen *pUnipen, sigDelineation *sig_del, sigSignal *sig
	,int s_start, int s_end)
{
	int i,j,n,s,penlift,samplenr,streamnr;
	int str_start,str_end;
	int sam_start,sam_end;
	sigDelineation *del;
	sigCharStream *streams;

	if (s_start>s_end) {
		fprintf (stderr,"are you kiddin'? start %d must be <= end %d\n",s_start,s_end);
		return NULL;
	}
	if ( (s_start>=sig->nsamples) || (s_end>=sig->nsamples) ) {
		fprintf (stderr,"ERROR: delineation %d-%d must be within [0-%d]!\n"
			,s_start,s_end,sig->nsamples);
		return NULL;
	}

	if ((streams=sigDelineation2Charstream (pUnipen,sig_del))==NULL)
		return NULL;
	streamnr = sig_del->delineations[0].first_streamnr;
	penlift = (sig->z[0]>0); /* may not be good .... */
	j = sig_del->delineations[0].first_samplenr;


	/* search for the delineation containing 's_start' */
	/* let 's' always point into the parent signal */

	samplenr = 0;
	for (s=0;s<s_start;s++) {
		if (sig->z[s]!=-1) {
				samplenr++;
		}
	}
	j = samplenr;
	i = 0;
	while (j>=streams->nsamples[i]) {
		j -= streams->nsamples[i++];
	}
	str_start = i + sig_del->delineations[0].first_streamnr;
	sam_start = j;

	/* search for the delineation containing 's_end' */

	for (;s<s_end;s++) {
		if (sig->z[s]!=-1) {
				samplenr++;
		}
	}
	j = samplenr;
	i = 0;
	while (j>=streams->nsamples[i]) {
		j -= streams->nsamples[i++];
	}
	str_end = i+sig_del->delineations[0].first_streamnr;
	sam_end = j;

	sigDeleteCharStream (streams);

	/* we now we have at least one delineation */
	up_alloc(del,sigDelineation,1);
	up_alloc(del->delineations,sigSampleDelineation,1);
	n = del->ndels = 1;

	/* look for the delineation containing 'str_start:sam_start' */
	for (i=0;i<sig_del->ndels;i++) {
		if (delineation_contains(&(sig_del->delineations[i]),str_start,sam_start)) {
			del->delineations[0].first_streamnr = str_start;
			if (str_start==sig_del->delineations[i].first_streamnr)
				del->delineations[0].first_samplenr = sig_del->delineations[i].first_samplenr+sam_start;
			else
				del->delineations[0].first_samplenr = sam_start;
			if (delineation_contains(&(sig_del->delineations[i]),str_end,sam_end)) {
				del->delineations[0].last_streamnr = str_end;
				if (str_end==sig_del->delineations[i].first_streamnr)
					del->delineations[0].last_samplenr = sig_del->delineations[i].first_samplenr+sam_end;
				else
					del->delineations[0].last_samplenr = sam_end;
				return del;
			}
			else {
				del->delineations[0].last_streamnr = sig_del->delineations[i].last_streamnr;
				del->delineations[0].last_samplenr = sig_del->delineations[i].last_samplenr;
			}
			break;
		}
	}

	/* append any delineations up to 'str_end:sam_end' */
	i++;
	for (;i<sig_del->ndels;i++) {
		up_realloc(del->delineations,sigSampleDelineation,n+1);
		del->delineations[n].first_streamnr = sig_del->delineations[i].first_streamnr;
		del->delineations[n].first_samplenr = sig_del->delineations[i].first_samplenr;
		del->delineations[n].last_streamnr  = sig_del->delineations[i].last_streamnr;
		del->delineations[n].last_samplenr  = sig_del->delineations[i].last_samplenr;
		n++;
		del->ndels = n;
		if (delineation_contains(&(sig_del->delineations[i]),str_end,sam_end)) {
				del->delineations[n-1].last_streamnr = str_end;
				if (str_end==sig_del->delineations[i].first_streamnr)
					del->delineations[n-1].last_samplenr = sig_del->delineations[i].first_samplenr+sam_end;
				else
					del->delineations[n-1].last_samplenr = sam_end;
				return del;
		}
	}

	return del;
}

void upSkipSegment (tUPUnipen *pUnipen, tUPEntry *Entry, char *level)
{
	tUPEntry *next_entry;
	int streamnr;

	if (mBoundedDelineation(pUnipen))
		return;
		
	next_entry = upSearchNextSegmentEntry (pUnipen,level,NULL);
	streamnr = pUnipen->last_pen_stream+1;
	/* search the next segment */
	if (next_entry==NULL) { /* this is the last segment */
		pUnipen->last_pen_stream = pUnipen->NrOfPenStreams-1;
		return;
	}
	if (next_entry->Count-Entry->Count<2) {
		pUnipen->last_pen_stream = pUnipen->NrOfPenStreams-1;
		return;
	}
	while (pUnipen->stream_sequence[streamnr+1].entry->Count<next_entry->Count)
			streamnr++;
	pUnipen->last_pen_stream = streamnr;
}


static sigDelineation *count_delineation (tUPUnipen *pUnipen, tUPEntry *Entry, char *level)
{
	sigDelineation *del;
	tUPEntry *next_entry;
	int streamnr;

	next_entry = upSearchNextSegmentEntry (pUnipen,level,Entry);
	
	up_alloc(del,sigDelineation,1);
	up_alloc(del->delineations,sigSampleDelineation,1);
	del->delineations[0].first_streamnr = pUnipen->last_pen_stream+1;
	del->delineations[0].first_samplenr = -1;
	del->delineations[0].last_samplenr = -1;
	
	/* search the next segment */
	if (next_entry==NULL) { /* this is the last segment */
		del->delineations[0].last_streamnr = pUnipen->NrOfPenStreams-1;
	}
	else {
		if (next_entry->Count <= Entry->Count ) {
			fprintf (stderr,"ERROR: not enough streams for entry %d: %s!!\n",Entry->Count,Entry->Entry);
			fprintf (stderr,"occurring when searching for next_entry with count %d: %s after %s at %d\n"
				,next_entry->Count, next_entry->Entry
				,Entry->Entry, Entry->Count);
			return NULL;
		}
		streamnr = del->delineations[0].first_streamnr;
		while (((streamnr + 1) < pUnipen->NrOfPenStreams) &&
		       (pUnipen->stream_sequence[streamnr+1].entry->Count
		        < next_entry->Count)) {
			streamnr++;
		}
		del->delineations[0].last_streamnr = streamnr;
		pUnipen->last_pen_stream = streamnr;
	}

        if(logVerbosity(pUnipen) > 0) {
	    fprintf (stderr,"%%up count delin segm=%s at entry=%d stream=%d thru %d \n"
				,Entry->Entry
				,Entry->Count
				,del->delineations[0].first_streamnr
				,del->delineations[0].last_streamnr);
			
	    fflush(stderr);
        }
	


	del->ndels = 1;
	return del;
}

int upNSubStrings (char *s)
{
	int n = 0;
	/* skip until the first non-scheider */
	while (mIsScheider(*s)&&*s!='\0') s++;
	/* skip until n==0 */
	while (*s!='\0') {
		n++;
		/* skip until the first scheider */
		while (!mIsScheider(*s)&&*s!='\0') s++;
		if (*s=='\0') return n;
		/* and skip until the first non-scheider */
		while (mIsScheider(*s)&&*s!='\0') s++;
		if (*s=='\0') return n;
	}
	return n;
}

void sigFreeDelineation (sigDelineation *del)
{
	free(del->delineations);
	free(del);
}

void sigDeleteCharStream (sigCharStream *s)
{
	int i;

	for (i=0;i<s->nstreams;i++) {
		free(s->streams[i]);
	}
	free(s->streams);
	free(s->nsamples);
}

#ifdef TRY_COMPRESSING_DELINEATIONS
/* if one of the situations below occurs, join the two delineations!
*
*     s1<=s2 && e1>=e2     s1------------------e1   delete s2
*                                   s2----e2
*
*     s2<=s1 && e2>=e1     s1-------------e1        delete s1
*                        s2------------------e2
*
*     e1>=s2 && e1<=e2   s1------------e1           create s1-e2
*                             s2------------e2
*
*     e2>=s1 && e1>=e2      s1------------e1        create s2-e1
*                       s2------------e2
*/

void compress_delineation (tUPUnipen *pUnipen,sigDelineation *del)
{
		tUPStream *sequences = pUnipen->stream_sequence;
		int *valid_streams   = pUnipen->valid_streams;
		int i;
		int s_nr1,s_nr2; /* stream numbers */
		int s1,s2;       /* sample starts  */
		int e1,e2;       /* sample ends    */

		for (i=0;i<del->ndels-1;i++) {
			s_nr1 = del->delineations[i].first_streamnr;
			s_nr1 = valid_streams[s_nr1];
			s_nr2 = del->delineations[i+1].first_streamnr;
			s_nr2 = valid_streams[s_nr2];
			s1    = sequences[s_nr1].first_sample;
			e1    = s1 + sequences[s_nr1].nsamples;
			s2    = sequences[s_nr2].first_sample;
			e2    = s2 + sequences[s_nr2].nsamples;
			if (s1<=s2 && e1>=e2) {
				delete_delineation(i+1,del);
			} else if (s2<=s1 && e2>=e1) {
			} else if (e1>=s2 && e1<=e2) {
			} else if (e2>=s1 && e1>=e2) {
			}
		}
}
#endif

void adjust_delineation (tUPUnipen *pUnipen, sigDelineation *del)
{
	tUPStream *stream;
	int firststr,firstsam,laststr,lastsam;
	int i;

	for (i=0;i<del->ndels;i++) {
		firststr = del->delineations[i].first_streamnr;
		firstsam = del->delineations[i].first_samplenr;
		laststr  = del->delineations[i].last_streamnr;
		lastsam  = del->delineations[i].last_samplenr;
		if (firststr==-1) {
			fprintf (stderr,"THIS CANNOT HAPPEN! FIRST STREAMNR = -1!!\n");
			return;
		}
		if (firstsam<0) {
			firstsam=0;
		}
		if (laststr==-1) {
			laststr = firststr;
			stream = &pUnipen->stream_sequence[pUnipen->valid_streams[laststr]];
			lastsam = stream->nsamples-1;
		} else if (lastsam==-1) {
			stream = &pUnipen->stream_sequence[pUnipen->valid_streams[laststr]];
			lastsam = stream->nsamples-1;
		}
		if (lastsam<0)
			lastsam = 0;
		del->delineations[i].first_streamnr = firststr;
		del->delineations[i].first_samplenr = firstsam;
		del->delineations[i].last_streamnr = laststr ;
		del->delineations[i].last_samplenr = lastsam ;
	}
}

sigDelineation *sigEntry2Delineation (tUPUnipen *pUnipen, tUPEntry *Entry)
{
	sigDelineation *del=NULL;
	char level[128],delineation[4096];

	/* entry has .SEGMENT LEVEL DELINEATION blabla */
	if (sscanf(Entry->Entry,".SEGMENT%s%s",level,delineation)!=2) {
		fprintf (stderr,"unable to scan level and delineation from entry:\n%s\n",Entry->Entry);
		
	}
	del = sigParseDelineation (delineation);
	if (del == NULL) { /* assuming an unknown delineation */
		mUnknownDelineation(pUnipen) = 1;
		if (mBoundedDelineation(pUnipen)) {
			fprintf (stderr,"cannot handle both unknown and bounded delineations!\n");
			exit (1);
		}
		del = count_delineation(pUnipen, Entry, level);
	}
	else {
#ifdef TRY_COMPRESSING_DELINEATIONS
		compress_delineation (pUnipen, del);
#endif
		mBoundedDelineation(pUnipen) = 1;
	}
	if (del != NULL)
		adjust_delineation(pUnipen,del);
		
	return del;
}


sigCharStream *sigGetSamples (tUPUnipen *pUnipen, tUPEntry *Entry)
{
	sigCharStream *result;
	sigDelineation *del;

	del = sigEntry2Delineation(pUnipen,Entry);
	result = sigDelineation2Charstream (pUnipen,del);
	sigFreeDelineation(del);
	return result;
}

sigCharStream *sigNextSegmentSamples (tUPUnipen *pUnipen, char *level)
{
	tUPEntry *entry;

	entry = upNextSegment(pUnipen,level,1);
	if (entry == NULL)
		return NULL;
			
	return sigGetSamples(pUnipen,entry);
}

int sigSegmentInSegment (sigDelineation *del1, sigDelineation *del2)
{
	int p1_start,p1_end,p2_start,p2_end; /* penstream numbers */
	int s1_start,s1_end,s2_start,s2_end; /* sample numbers    */
	int n;

	n = del1->ndels-1;
	p1_start = del1->delineations[0].first_streamnr;
	p1_end   = del1->delineations[n].last_streamnr;
	s1_start = del1->delineations[0].first_samplenr;
	s1_end   = del1->delineations[n].last_samplenr;

	n = del2->ndels-1;
	p2_start = del2->delineations[0].first_streamnr;
	p2_end   = del2->delineations[n].last_streamnr;
	s2_start = del2->delineations[0].first_samplenr;
	s2_end   = del2->delineations[n].last_samplenr;

	if (p1_start==-1||p2_start==-1||p1_end==-1||p2_end==-1) /* unknow delineation    */
		return 1;
	
	if (p1_start>p2_start || p1_end<p2_end)         /* obvious penstream mismatch    */
		return 0;

	if (p1_start<p2_start && p1_end>p2_end)         /* obvious penstream match       */
		return 1;

	/* now I am sure that the domain [p1_start,p1_end] >= [p2_start,p2_end]          */

	if (p1_start==p2_start&&s1_start>s2_start)   /* same stream, but sample mismatch */
		return 0;
	
	if (p1_end==p2_end&&s1_end<s2_end) /* same check, also check for numbering s1    */
		if (s1_end!=-1)
			return 0;	
#ifdef THOROUGH_DELINEATION_CHECK /* not finished so ......... */
	/* now check for sample mismatches in each pen-stream */

	for (i=0;i<del2->ndels;i++) {
		p2_start = del2->delineations[i].first_streamnr;
		p2_end   = del2->delineations[i].last_streamnr;
		s2_start = del2->delineations[i].first_samplenr;
		s2_end   = del2->delineations[i].last_samplenr;
		for (j=0;j<del1->ndels;j++) { /* search for a surrounding delineation */
			p1_start = del1->delineations[j].first_streamnr;
			p1_end   = del1->delineations[j].last_streamnr;
			s1_start = del1->delineations[j].first_samplenr;
			s1_end   = del1->delineations[j].last_samplenr;
			if (p1_start<=p2_start && s1_start<=s2_start) { /* found a potential lower-bound */
		}
	}
#endif
	return 1;
}

tUPEntry **sigSegmentsInSegment (tUPUnipen *pUnipen, tUPEntry *Entry, char *level, int *nsegments)
{
	int cur_segment = mCurrOffsetSegment(pUnipen);
	sigDelineation *del1 = sigEntry2Delineation(pUnipen,Entry),*del2;
	tUPEntry **entries=NULL,*entry;
	int n = 0;
	
	while ((entry=upNextSegment(pUnipen,level,0))!=NULL) {
		del2 = sigEntry2Delineation(pUnipen,entry);
		if (sigSegmentInSegment(del1,del2)) {
			if (n==0) {
				up_alloc(entries,tUPEntry *,1);
			}
			else {
				up_realloc(entries,tUPEntry *,n+1);
			}
			entries[n++] = entry;
			sigFreeDelineation(del2);
		}
		else {
			sigFreeDelineation(del2);
			break;
		}
	}
	sigFreeDelineation(del1);
	mCurrOffsetSegment(pUnipen) = cur_segment;
	*nsegments = n;
	return entries;
}

/******************** SIGNAL ******************/

/* converts an ascii set of penstreams to upSignal, i.e.
   x,y,z coordinates

   NOTE: for 2 subsequent .PEN_DOWN movements,
   a 150 ms linear trajectory is inserted!!!!!!!!!
*/

int sigInsertLinearTrajectory (sigSignal *usignal, double samplerate, int cur_sample, int x, int y, int x2, int y2)
{
	double dx,dy;
	int s,j,nsamples = (int) (T_INSERTION*samplerate);

	if (nsamples<2) {
		fprintf (stderr,"sigInsertLinearTrajectory: samplerate (%f) leads to ZERO insertions, taking one sample with z=-1\n",samplerate);
		nsamples = 2;
	}
	usignal->nsamples += nsamples-1;
	up_realloc(usignal->x,int,usignal->nsamples);
	up_realloc(usignal->y,int,usignal->nsamples);
	up_realloc(usignal->z,int,usignal->nsamples);
	dx = ((double)(x2-x))/nsamples;
	dy = ((double)(y2-y))/nsamples;
	for (j=1,s=cur_sample;j<nsamples;s++,j++) {
		usignal->x[s] = (int) (j*dx)+x;
		usignal->y[s] = (int) (j*dy)+y;
		usignal->z[s] = cPenAdded;
	}
	return s;
}

int sigStreamType (char *s)
{
	char  stream_type[128];

	if (sscanf(s,"%s",stream_type)!=1) {
		fprintf (stderr,"UNABLE TO DETERMINE STREAMTYPE in %s!!\n",s);
		return INVALID_STREAMTYPE;
	}
	if (strcmp(stream_type,keyPenLapse)==0)
		return DT;
	if (strcmp(stream_type,keyPenUp)==0)
		return PEN_UP;
	if (strcmp(stream_type,keyPenDown)==0)
		return PEN_DOWN;
	fprintf (stderr,"INVALID STREAMTYPE %s!!\n",stream_type);
	return INVALID_STREAMTYPE;
}

char *get_sample_from_string (tUPUnipen *pUnipen, char *ptr, int *x, int *y, int *z)
{
	char *newptr = ptr;
	int i;

	*x = (int) strtol(ptr,&newptr,10);
	if (ptr==newptr) {
		return NULL;
	}
	ptr = newptr;
	*y = (int) strtol(ptr,&newptr,10);
	if (ptr==newptr) {
		return NULL;
	}
	/* now we may search for Z, first checking for Z, else P */
	i = 2;
	if (pUnipen->has_z!=-1) {
		for (i=2;i<pUnipen->NrOfCoords;i++) {
			ptr = newptr;
			*z = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr) {
				return NULL;
			}
			if (i==pUnipen->has_z) {
				i++;
				break;
			}
		}
	} else { if (pUnipen->has_p!=-1) {
		for (i=2;i<pUnipen->NrOfCoords;i++) {
			ptr = newptr;
			*z = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr) {
				return NULL;
			}
			if (i==pUnipen->has_p) {
				i++;
				break;
			}
		}
	} }
	/* skip the rest */
	for (;i<pUnipen->NrOfCoords;i++) {
		ptr = newptr;
		(void) strtol(ptr,&newptr,10);
		if (ptr==newptr) {
			return NULL;
		}
	}
	return newptr;
}

int sigDelineation2CharSignal (tUPUnipen *pUnipen, sigDelineation *del
	,int *onsamples, int **ox, int **oy, int **oz)
{
	sigCharStream *streams = sigDelineation2Charstream(pUnipen,del);
	int ret_code;

	ret_code = sigCharStream2CharSignal(pUnipen,streams,onsamples,ox,oy,oz);
	sigDeleteCharStream(streams);
	return ret_code;
}

int sigCharStream2CharSignal (tUPUnipen *pUnipen, sigCharStream *streams
	,int *onsamples, int **ox, int **oy, int **oz)
{
	int cur_sample = 0,nsamples = 0;
	int i,s,pen_isup=-1;
	int x,y,z;
	char *ptr;

	for (i=0;i<streams->nstreams;i++) {
		nsamples += streams->nsamples[i];
	}
	*onsamples = nsamples;
	if (nsamples>0) {
		up_alloc(*ox,int,nsamples);
		up_alloc(*oy,int,nsamples);
		up_alloc(*oz,int,nsamples);
	}

	for (i=0;i<streams->nstreams;i++) {
		ptr = streams->streams[i];
		switch (sigStreamType(ptr)) {
			case PEN_UP:
				ptr = upNthString(ptr,1);
				pen_isup = 1;
				for (s=cur_sample;s<cur_sample+streams->nsamples[i];s++) {
					z = 0;
					ptr = get_sample_from_string (pUnipen,ptr,&x,&y,&z);
					(*ox)[s] = x;
					(*oy)[s] = y;
					(*oz)[s] = z;
					if (ptr==NULL)
						break;
				}
				cur_sample = s;
				break;
			case PEN_DOWN:
				ptr = upNthString(ptr,1);
				pen_isup = 0;
				z = AX_PEN_FORCE; /* simulated pen on paper, axial pen-force in grams */
				for (s=cur_sample;s<cur_sample+streams->nsamples[i];s++) {
					ptr = get_sample_from_string (pUnipen,ptr,&x,&y,&z);
					(*ox)[s] = x;
					(*oy)[s] = y;
					(*oz)[s] = z;
					if (ptr==NULL)
						break;
				}
				cur_sample = s;
				break;
			case DT:
				break;
			default:
				fprintf (stderr,"invalid streamtype: %s\n",streams->streams[i]);
				exit (1);
		}
	}
	return 1;
}

int sigEntry2CharSignal (tUPUnipen *pUnipen, tUPEntry *Entry
	,int *onsamples, int **ox, int **oy, int **oz)
{
	sigCharStream *streams = sigGetSamples(pUnipen,Entry);
	int cur_sample = 0,nsamples = 0;
	int i,s,pen_isup=-1;
	int x,y,z;
	char *ptr;

	for (i=0;i<streams->nstreams;i++) {
		nsamples += streams->nsamples[i];
	}
	*onsamples = nsamples;
	if (nsamples>0) {
		up_alloc(*ox,int,nsamples);
		up_alloc(*oy,int,nsamples);
		up_alloc(*oz,int,nsamples);
	}

	for (i=0;i<streams->nstreams;i++) {
		ptr = streams->streams[i];
		switch (sigStreamType(ptr)) {
			case PEN_UP:
				ptr = upNthString(ptr,1);
				pen_isup = 1;
				for (s=cur_sample;s<cur_sample+streams->nsamples[i];s++) {
					z = 0;
					ptr = get_sample_from_string (pUnipen,ptr,&x,&y,&z);
					(*ox)[s] = x;
					(*oy)[s] = y;
					(*oz)[s] = z;
					if (ptr==NULL)
						break;
				}
				cur_sample = s;
				break;
			case PEN_DOWN:
				ptr = upNthString(ptr,1);
				pen_isup = 0;
				z = AX_PEN_FORCE; /* simulated pen on paper, axial pen-force in grams */
				for (s=cur_sample;s<cur_sample+streams->nsamples[i];s++) {
					ptr = get_sample_from_string (pUnipen,ptr,&x,&y,&z);
					(*ox)[s] = x;
					(*oy)[s] = y;
					(*oz)[s] = z;
					if (ptr==NULL)
						break;
				}
				cur_sample = s;
				break;
			case DT:
				break;
			default:
				fprintf (stderr,"invalid streamtype: %s\n",streams->streams[i]);
				exit (1);
		}
	}

	sigDeleteCharStream(streams);
	return 1;
}

sigSignal *remove_penup_head_tail (sigSignal *usig, double threshold)
{
	int i,ns,offset;
	sigSignal *sig;

   /* remove pen_up head */
	offset = 0;
	ns = usig->nsamples;
	while (usig->z[offset]<threshold) {
		if (offset==ns-1) {
			fprintf (stderr,"returning penup segment!\n");
			return NULL;
		}
		offset++;
	}
	ns = usig->nsamples-1;
	while (usig->z[ns]<threshold&&ns>offset)
		ns--;
	ns -= offset;

	up_alloc(sig,sigSignal,1);
	sig->nsamples = ns+1;
	up_alloc(sig->x,int,ns+1);
	up_alloc(sig->y,int,ns+1);
	up_alloc(sig->z,int,ns+1);
	for (i=0;i<ns+1;i++) {
		sig->x[i] = usig->x[i+offset];
		sig->y[i] = usig->y[i+offset];
		sig->z[i] = usig->z[i+offset];
	}
	return sig;
}

sigSignal *sigEntry2CleanSignal (tUPUnipen *pUnipen, tUPEntry *Entry
	, int mode, double threshold)
{
	sigSignal *usignal,*clean_signal;
	sigCharStream *streams;
	
	streams = sigGetSamples(pUnipen,Entry);
	usignal = sigCharstream2Signal (pUnipen, Entry, mode, streams);
	clean_signal = remove_penup_head_tail(usignal,threshold);
	sigDeleteCharStream(streams);
	if (clean_signal==NULL)
		return usignal;
	sigDeleteSignal(usignal);
	return clean_signal;
}

sigSignal *sigEntry2Signal (tUPUnipen *pUnipen, tUPEntry *Entry, int mode)
{
	sigSignal *usignal;
	sigCharStream *streams;
	
	streams = sigGetSamples(pUnipen,Entry);
	usignal = sigCharstream2Signal (pUnipen, Entry, mode, streams);
	sigDeleteCharStream(streams);
	return usignal;
}

sigSignal *sigDelineation2Signal (tUPUnipen *pUnipen, sigDelineation *del)
{
	sigCharStream *cstream;
	sigSignal *sig;

	if ((cstream=sigDelineation2Charstream (pUnipen,del))==NULL) {
		return NULL;
	}
	sig = sigCharstream2Signal(pUnipen,NULL,TIME_EQUI_DIST,cstream);
	sigDeleteCharStream(cstream);

	return sig;
}

sigSignal *sigCharstream2Signal (tUPUnipen *pUnipen, tUPEntry *Entry, int mode, sigCharStream *streams)
{
	int cur_sample = 0,nsamples = 0;
	int i,s,have_to_insert_DT = 0,pen_isup=-1;
	int x,y,z,x2,y2;
	double samplerate;
	sigSignal *usignal;
	char *ptr;

	samplerate = 100.0;

	if (Entry!=NULL) {
		if (pUnipen->Entries[pUnipen->SampleRateId]==NULL) {
			samplerate = 100.0;
		} else if (sscanf(mSampleRate(pUnipen)->Entry,".POINTS_PER_SECOND%lf",&samplerate)!=1) {
			samplerate = 100.0;
		} else if (samplerate<1||samplerate>500) {
			fprintf (stderr,"INVALID SAMPLERATE (%f Hz), setting it to 100 Hz!\n",samplerate);
			samplerate = 100.0;
		}
	}

	for (i=0;i<streams->nstreams;i++) {
		nsamples += streams->nsamples[i];
	}
	up_alloc(usignal,sigSignal,1);
	usignal->nsamples = nsamples;
	if (nsamples>0) {
		up_alloc(usignal->x,int,nsamples);
		up_alloc(usignal->y,int,nsamples);
		up_alloc(usignal->z,int,nsamples);
	}

	switch (mode) {
	case TIME_EQUI_DIST:
		for (i=0;i<streams->nstreams;i++) {
			ptr = streams->streams[i];
			switch (sigStreamType(ptr)) {
				case PEN_UP:
					ptr = upNthString(ptr,1);
					if (have_to_insert_DT) {
						have_to_insert_DT = 0;
						(void) get_sample_from_string (pUnipen,ptr,&x2,&y2,&z);
						cur_sample = sigInsertLinearTrajectory(usignal,samplerate,cur_sample,x,y,x2,y2);
					}
					pen_isup = 1;
					for (s=cur_sample;s<cur_sample+streams->nsamples[i];s++) {
						z = 0;
						ptr = get_sample_from_string (pUnipen,ptr,&x,&y,&z);
						usignal->x[s] = x;
						usignal->y[s] = y;
						usignal->z[s] = z;
						if (ptr==NULL)
							break;
					}
					cur_sample = s;
					break;
				case PEN_DOWN:
					ptr = upNthString(ptr,1);
					if (have_to_insert_DT) {
						have_to_insert_DT = 0;
						(void) get_sample_from_string (pUnipen,ptr,&x2,&y2,&z);
						cur_sample = sigInsertLinearTrajectory(usignal,samplerate,cur_sample,x,y,x2,y2);
						pen_isup = 1;
					}
					if (pen_isup==0) { /* previous was also a .PEN_DOWN, insert trajectory */
						(void) get_sample_from_string (pUnipen,ptr,&x2,&y2,&z);
						cur_sample = sigInsertLinearTrajectory(usignal,samplerate,cur_sample,x,y,x2,y2);
					}
					pen_isup = 0;
					z = AX_PEN_FORCE; /* simulated pen on paper, axial pen-force in grams */
					for (s=cur_sample;s<cur_sample+streams->nsamples[i];s++) {
						ptr = get_sample_from_string (pUnipen,ptr,&x,&y,&z);
						usignal->x[s] = x;
						usignal->y[s] = y;
						usignal->z[s] = z;
						if (ptr==NULL)
							break;
					}
					cur_sample = s;
					break;
				case DT:
					have_to_insert_DT = 1;
					break;
				default:
					fprintf (stderr,"invalid streamtype: %s\n",streams->streams[i]);
					exit (1);
			}
		}
		break;
	case SPACE_EQUI_DIST:
		break;
	case RAW_SIGNAL:
		break;
	default:
		break;
	}
	usignal->nsamples = cur_sample;
	return usignal;
}

void sigPrintSignal (sigSignal *usignal)
{
	int i;
	for (i=0;i<usignal->nsamples;i++)
		printf ("%d %d %d\n",usignal->x[i],usignal->y[i],usignal->z[i]);
}

void sigDeleteSignal (sigSignal *usignal)
{
	if (usignal->nsamples>0) {
		free(usignal->x);
		free(usignal->y);
		free(usignal->z);
	}
	free(usignal);
}

#ifdef OLD_NOT_SO_STRICT
void sigSegmentBoundsInSignal (tUPUnipen *pUnipen, tUPEntry *signal_entry,tUPEntry *segment_entry,int *offset, int *nsamples)
{
	sigCharStream *seg_stream,*sig_stream;
	sigDelineation *seg_del,*sig_del;
	sigSignal *seg_signal,*sig_signal;
	int last_streamnr,last_samplenr;

	seg_del = sigEntry2Delineation(pUnipen,segment_entry);
	sig_del = sigEntry2Delineation(pUnipen,signal_entry);
	seg_stream = sigDelineation2Charstream (pUnipen,seg_del);
	seg_signal = sigCharstream2Signal (pUnipen, segment_entry, TIME_EQUI_DIST, seg_stream);
	*nsamples = seg_signal->nsamples;
	
	last_streamnr = seg_del->delineations[0].first_streamnr; /* to use as end of signal */
	last_samplenr = seg_del->delineations[0].first_samplenr;
	sigDeleteSignal(seg_signal);
	sigFreeDelineation(seg_del);
	sigDeleteCharStream(seg_stream);

	sig_del->ndels = 1;
	sig_del->delineations[0].last_streamnr = last_streamnr;
	sig_del->delineations[0].last_samplenr = last_samplenr;
	sig_stream = sigDelineation2Charstream (pUnipen,sig_del);
	sig_signal = sigCharstream2Signal (pUnipen, signal_entry, TIME_EQUI_DIST, sig_stream);
	*offset = sig_signal->nsamples;

	sigDeleteSignal(sig_signal);
	sigFreeDelineation(sig_del);
	sigDeleteCharStream(sig_stream);
}
#else
void sigSegmentBoundsInSignal (tUPUnipen *pUnipen, tUPEntry *signal_entry,tUPEntry *segment_entry,int *offset, int *nsamples)
{
	sigCharStream *sig_stream;
	sigDelineation *seg_del,*sig_del;
	sigSignal *sig_signal;
	int first_streamnr,first_samplenr,last_streamnr,last_samplenr;
	int s_nr,ndels;

	sig_del = sigEntry2Delineation(pUnipen,signal_entry);
	seg_del = sigEntry2Delineation(pUnipen,segment_entry);
	ndels = seg_del->ndels-1;
	first_streamnr = seg_del->delineations[0].first_streamnr;
	first_samplenr = seg_del->delineations[0].first_samplenr;
	last_streamnr = seg_del->delineations[ndels].last_streamnr;
	last_samplenr = seg_del->delineations[ndels].last_samplenr;
	sigFreeDelineation(seg_del);

	ndels = sig_del->ndels;
	/* compute total trajectory in sig from seg by finding seg-end */
	for (s_nr=0;s_nr<sig_del->ndels;s_nr++) {
		if (sig_del->delineations[s_nr].last_streamnr>=last_streamnr) {
			sig_del->delineations[s_nr].last_streamnr = last_streamnr;
			sig_del->delineations[s_nr].last_samplenr = last_samplenr;
			sig_del->ndels = s_nr+1;
			break;
		}
	}
	if (s_nr==sig_del->ndels) {
		sig_del->delineations[s_nr-1].last_streamnr = last_streamnr;
		sig_del->delineations[s_nr-1].last_samplenr = last_samplenr;
	}
	if ((sig_stream=sigDelineation2Charstream (pUnipen,sig_del))==NULL) {
		return;
	}
	sig_signal = sigCharstream2Signal (pUnipen, signal_entry, TIME_EQUI_DIST, sig_stream);
	*nsamples = sig_signal->nsamples;
	sigDeleteSignal(sig_signal);
	sigDeleteCharStream(sig_stream);

	sig_del->ndels = ndels;
	for (s_nr=0;s_nr<sig_del->ndels;s_nr++) {
		if (sig_del->delineations[s_nr].first_streamnr>=first_streamnr) {
			sig_del->delineations[s_nr].last_streamnr = first_streamnr;
			sig_del->delineations[s_nr].last_samplenr = first_samplenr;
			sig_del->ndels = s_nr+1;
			break;
		}
	}
	if (s_nr==sig_del->ndels) {
		sig_del->delineations[s_nr-1].last_streamnr = first_streamnr;
		sig_del->delineations[s_nr-1].last_samplenr = first_samplenr;
	}
	if ((sig_stream=sigDelineation2Charstream (pUnipen,sig_del))==NULL) {
		return;
	}
	sig_signal = sigCharstream2Signal (pUnipen, signal_entry, TIME_EQUI_DIST, sig_stream);
	*offset = sig_signal->nsamples - 1;

	*nsamples -= *offset;

	sig_del->ndels = ndels;
	sigDeleteSignal(sig_signal);
	sigFreeDelineation(sig_del);
	sigDeleteCharStream(sig_stream);
}
#endif

void sigSegmentBoundsInDelineation (tUPUnipen *pUnipen, sigDelineation  *d1, sigDelineation *d2
	,int *offset, int *nsamples)
{
	sigCharStream *sig_stream;
	sigDelineation *sig_del;
	sigSignal *sig_signal;
	int first_streamnr,first_samplenr,last_streamnr,last_samplenr;
	int s_nr,ndels;
	int ok_to_go_on;

	sig_del = sigCopyDelineation(d1);

	ndels = d2->ndels-1;
	first_streamnr = d2->delineations[0].first_streamnr;
	first_samplenr = d2->delineations[0].first_samplenr;
	last_streamnr = d2->delineations[ndels].last_streamnr;
	last_samplenr = d2->delineations[ndels].last_samplenr;

	ok_to_go_on = 1;
	ndels = sig_del->ndels;
	/* compute total trajectory in sig from seg by finding seg-end */
	for (s_nr=0;s_nr<ndels&&ok_to_go_on;s_nr++) {
		if (sig_del->delineations[s_nr].first_streamnr>last_streamnr) {
			sig_del->ndels = s_nr + 1;
			ok_to_go_on = 0;
		}
		if (sig_del->delineations[s_nr].first_streamnr==last_streamnr) {
			if (sig_del->delineations[s_nr].first_samplenr>last_samplenr) {
				sig_del->delineations[s_nr].first_samplenr = last_samplenr;
			}
			sig_del->ndels = s_nr + 1;
			ok_to_go_on = 0;
		}
		if (sig_del->delineations[s_nr].last_streamnr>=last_streamnr) {
			sig_del->delineations[s_nr].last_samplenr = last_samplenr;
			sig_del->delineations[s_nr].last_streamnr = last_streamnr;
			sig_del->ndels = s_nr + 1;
			ok_to_go_on = 0;
		}
	}
	sig_del->ndels = s_nr;

	if ((sig_stream=sigDelineation2Charstream (pUnipen,sig_del))==NULL) {
		return;
	}

	if ((sig_signal=sigCharstream2Signal(pUnipen,NULL,TIME_EQUI_DIST,sig_stream))==NULL) {
		fprintf (stderr,"unable to get signal from charstream!!\n");
		fprintf (stderr,"error: %d charstreams\n",sig_stream->nstreams);
		for (s_nr=0;s_nr<sig_stream->nstreams;s_nr++)
			fprintf (stderr,"%d %s\n",sig_stream->nsamples[s_nr],sig_stream->streams[s_nr]);
		exit(1);
	}
	*nsamples = sig_signal->nsamples;
	sigDeleteSignal(sig_signal);
	sigDeleteCharStream(sig_stream);

	sigFreeDelineation(sig_del);
	sig_del = sigCopyDelineation(d1);

	ndels = sig_del->ndels;
	/* compute total trajectory in sig from 0 by finding seg-start */
	for (s_nr=0;s_nr<ndels;s_nr++) {
		if (sig_del->delineations[s_nr].first_streamnr>first_streamnr) {
			sig_del->ndels = s_nr;
			break;
		}
		if (sig_del->delineations[s_nr].first_streamnr==first_streamnr) {
			if (sig_del->delineations[s_nr].first_samplenr>first_samplenr) {
				sig_del->ndels = s_nr;
				break;
			} else {
				sig_del->ndels = s_nr + 1;
				sig_del->delineations[s_nr].last_samplenr = first_samplenr;
				sig_del->delineations[s_nr].last_streamnr = first_streamnr;
				break;
			}
		}
		if (sig_del->delineations[s_nr].last_streamnr>first_streamnr) {
			sig_del->delineations[s_nr].last_samplenr = first_samplenr;
			sig_del->delineations[s_nr].last_streamnr = first_streamnr;
			sig_del->ndels = s_nr + 1;
			break;
		}
		if (sig_del->delineations[s_nr].last_streamnr==first_streamnr) {
			if (sig_del->delineations[s_nr].last_samplenr>first_samplenr) {
				sig_del->delineations[s_nr].last_samplenr = first_samplenr;
			}
			sig_del->ndels = s_nr + 1;
			break;
		}
	}
	if ((sig_stream=sigDelineation2Charstream (pUnipen,sig_del))==NULL) {
		return;
	}
	sig_signal = sigCharstream2Signal (pUnipen, NULL, TIME_EQUI_DIST, sig_stream);
	*offset = sig_signal->nsamples - 1;

	*nsamples -= *offset;

	sig_del->ndels = ndels;
	sigDeleteSignal(sig_signal);
	sigFreeDelineation(sig_del);
	sigDeleteCharStream(sig_stream);
}

sigSignal *sigPenstream2Signal(tUPUnipen *pUnipen, int first_stream, int last_stream)
{
	int i;
	sigSignal *s;
	sigCharStream cstream;

	up_alloc(cstream.streams,char *,pUnipen->nvalid_streams);
	up_alloc(cstream.nsamples,int,pUnipen->nvalid_streams);
	for (i=0;i<pUnipen->nvalid_streams;i++) {
		cstream.streams[i] = pUnipen->stream_sequence[i].entry->Entry;
		cstream.nsamples[i] = pUnipen->stream_sequence[i].nsamples;
	}
	cstream.nstreams = pUnipen->nvalid_streams;
	s = sigCharstream2Signal (pUnipen,NULL,TIME_EQUI_DIST,&cstream);
	free(cstream.streams);
	free(cstream.nsamples);
	return s;
}
