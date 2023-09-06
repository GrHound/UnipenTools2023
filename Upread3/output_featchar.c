#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>

/*
#include <vhs.h>
#include <vhs_proto.h>
#include <vhs_hclus.h>
*/

#include "output_featxy.h"
#include "allo_resample.h"
#include "writer_identifications.h"
#include "upread3.h"

void output_allograph (FILE *fp, char *name, int ns, int m, int *xi, int *yi, int *zi)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	double X[MAX_RESAMPLE_POINTS*3];
	int offset, i, no;

	/* remove pen_up head */
	offset = 0;
	while (zi[offset]<15.) {
		if (offset==ns-1) {
			fprintf (stderr,"skipping penup segment!\n");
			return;
		}
		offset++;
	}

	/* remove pen-up tail */
	ns = ns - 1;
	while (zi[ns]<15.&&ns>offset) {
		fprintf (stderr,"removing %d: %d %d %d\n",ns,xi[ns],yi[ns],zi[ns]);
		ns--;
	}

	ns -= offset;
	for (i=0;i<ns;i++) {
		x[i] = (float) xi[i+offset];
		y[i] = (float) yi[i+offset];
		z[i] = (float) zi[i+offset];
	}
	no = m;

	recog_spatial_z_sampler(x,y,z,ns,xs,ys,zs,no,va,0,ns-1,15.);
	for (i=0;i<no;i++) {
		X[3*i]   = (double) xs[i];
		X[3*i+1] = (double) ys[i];
		X[3*i+2] = (double) zs[i];
	}
	normalize_allo(X,no);
/*
	for (i=0;i<no;i++) {
		fprintf (stderr,"(%f,%f,%f) -> (%f,%f,%f)\n"
			,xs[i] ,ys[i] ,zs[i], X[3*i] ,X[3*i+1] ,X[3*i+2]);
	}
*/

	fprintf (fp,"%s",name);
	for (i=0;i<no*3;i++) {
		fprintf (fp," %f",X[i]);
	}
	fprintf (fp,"\n");
}

void init_output_featchar (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo)
{
	static int first = 1;
	int m;
	 
	if (!first)
		return;
	m = 3*oinfo->mreq_nsamples;
	if (oinfo->do_add_sincos)
		m += 2*(oinfo->mreq_nsamples-1);
	if (oinfo->do_add_phi)
		m += 2*(oinfo->mreq_nsamples-2);
	fprintf (fp_out,"NAMED DATA ??? %d\n",m);
	first = 0;
}

int output_featchar (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams)
{
	tUPEntry *entry,**segment_entries;
	int i,ns, *xi,*yi,*zi;
	char *name;

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		name = updirEntry2WriterCode(entry
			,entries
			,pUnipen->NrOfEntries[pUnipen->SegmentId]
			,oinfo->writer_code);
		fprintf (stderr,"outputing %d (%d,%d,%d) (%d,%d,%d)\n"
			,ns,xi[0],yi[0],zi[0],xi[ns-1],yi[ns-1],zi[ns-1]);
		output_allograph(fp_out,name,ns,oinfo->mreq_nsamples,xi,yi,zi);
		free(xi);
		free(yi);
		free(zi);
	}
	return 1;
}
