#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>

/*
#include <vhs.h>
#include <vhs_proto.h>
#include <vhs_hclus.h>
*/

#include "allo_resample.h"
#include "upstat1.h"

void output_contour_xy (FILE *fp, char *name, int ns, int m, int *xi, int *yi, int *zi, int named, int norm_pos_size)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	double X[MAX_RESAMPLE_POINTS*3];
	int offset, i, j, no;

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
	
        if(norm_pos_size == NORM_ALLO_POS_RADIUS) {
	    normalize_allo(X,no);
	} else if(norm_pos_size == NORM_LEFT) {
	    normalize_left(X,no);
	} else if(norm_pos_size == NORM_MIDDLE) {
	    normalize_middle(X,no);
	} else {
	    /* noop = NORM_RAW */
        }
/*
	for (i=0;i<no;i++) {
		fprintf (stderr,"(%f,%f,%f) -> (%f,%f,%f)\n"
			,xs[i] ,ys[i] ,zs[i], X[3*i] ,X[3*i+1] ,X[3*i+2]);
	}
*/	


	if(named) fprintf (fp,"%s ",name);
	j = 0;
	for (i=0;i<no;i++) {
		fprintf (fp,"%.3f %.3f  ",X[j], X[j+1]);
		j += 3;
	}
	fprintf (fp,"\n");
}

void init_output_featxy (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo)
{
	static int first = 1;
	int m;
	 
	if (!first)
		return;
	m = 3*oinfo->m;
	if (oinfo->do_add_sincos)
		m += 2*(oinfo->m-1);
	if (oinfo->do_add_phi)
		m += 2*(oinfo->m-2);
		
	fprintf (stderr,"Number of features (=x,y,x,y,...) %d\n",m);
	first = 0;
}

void clean_segment_name(char *segname)
{
   int last;
   last = strlen(segname)-1;
   if(last > 0) {
      if(segname[0] == '"') segname[0] = '-';
      if(segname[last] == '"') segname[last] = ' ';
   }
}


int output_featxy (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams
	, int named)
{
	tUPEntry *entry,**segment_entries;
	int i,ns, *xi,*yi,*zi;
	char name[2000];
	char segname[2000];
	char dum[2000];

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		sscanf(entry->Entry,"%s%s%s%s%s",dum,dum,dum,dum,segname);
		clean_segment_name(segname);
		
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		sprintf(name,"%s/%s",oinfo->writer_codestr,segname);
		fprintf (stderr,"outputing %d (%d,%d,%d) (%d,%d,%d)\n"
			,ns,xi[0],yi[0],zi[0],xi[ns-1],yi[ns-1],zi[ns-1]);
		output_contour_xy(fp_out,name,ns,oinfo->m,xi,yi,zi
		                        , named, oinfo->norm_pos_size);
		free(xi);
		free(yi);
		free(zi);
	}
	return 1;
}
