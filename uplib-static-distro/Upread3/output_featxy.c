/* output_featxy.c  Routine of upread3.c - Lambert Schomaker */

#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>
#include <math.h>

/*
#include <vhs.h>
#include <vhs_proto.h>
#include <vhs_hclus.h>
*/

#include "output_featxy.h"
#include "allo_resample.h"
#include "writer_identifications.h"
#include "upread3.h"

#define PRS_THRESHOLD 15. /* Artificial pressure threshold. If binary,
                             it will have been scaled to 0=penup 100=pendown,
                             which is ok. If from a pen-force transducer, it
                             will be 15 gf, which is also ok. */
			     
void shell_sortd(double darr[], int iarsiz,int ipoint[],int itype)
{
	register int more, i, j, igap, imax, kk;

	if(itype != 1 && itype != -1) {
              fprintf(stderr,"%%shell_sortd, itype not 1 or -1\n");
              exit(1);
	}

/* Initialize index array IPOINT. */

	for(i = 0; i < iarsiz; i++) {
	        ipoint[i] = i;
	}

	igap = iarsiz;
	while(igap > 1) {
	        igap = igap/2;
	        imax = iarsiz-igap;
	        more = 1;
	        while(more) {
	                more = 0;
	                for (i = 0; i < imax; i++) {
	                    j = i+igap;
	                    if((itype == 1 && 
	                      darr[ipoint[j]] < darr[ipoint[i]]) || 
	                       (itype == -1 && 
	                      darr[ipoint[j]] > darr[ipoint[i]])) {

	                                kk = ipoint[j];
	                                ipoint[j] = ipoint[i];
	                                ipoint[i] = kk;
	                                more = 1;
	                    }
	                }
	        }
	}
}
			    

void output_contour_xy (FILE *fp, char *name, int ns, int m
                        , int *xi, int *yi, int *zi, int named
                        , int norm_pos_size, Yfont *yfont)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	double X[MAX_RESAMPLE_POINTS*3];
	int offset, i, j, no, good_contour;

	/* remove pen_up head */
	offset = 0;
	while (zi[offset] < PRS_THRESHOLD) {
		if (offset==ns-1) {
			fprintf (stderr,"skipping penup segment!\n");
			return;
		}
		offset++;
	}

	/* remove pen-up tail */
	ns = ns - 1;
	while (zi[ns] < PRS_THRESHOLD&&ns>offset) {
		fprintf (stderr,"removing %d: %d %d %d\n",ns,xi[ns],yi[ns],zi[ns]);
		ns--;
	}

	ns -= offset;
	for (i=0;i < ns;i++) {
		x[i] = (float) xi[i+offset];
		y[i] = (float) yi[i+offset];
		z[i] = (float) zi[i+offset];
	}
	no = m;

	recog_spatial_z_sampler(x,y,z,ns,xs,ys,zs,no,va,0,ns-1,PRS_THRESHOLD);
	
	for (i=0;i<no;i++) {
		X[3*i]   = (double) xs[i];
		X[3*i+1] = (double) ys[i];
		X[3*i+2] = (double) zs[i];
	}
	
        if(norm_pos_size == NORM_ALLO_POS_RADIUS) {
	    normalize_allo(X,no);
	    good_contour = 1;
	} else if(norm_pos_size == NORM_LEFT) {
	    normalize_left(X,no);
	    good_contour = 1;
	} else if(norm_pos_size == NORM_MIDDLE) {
	    normalize_middle(X,no);
	    good_contour = 1;
	} else if(norm_pos_size == NORM_MID_X_BASELINE_Y) {
	    normalize_midx_basy(X,no,yfont,&good_contour);
	} else {
	    /* noop = NORM_RAW */
        }
/*
	for (i=0;i<no;i++) {
		fprintf (stderr,"(%f,%f,%f) -> (%f,%f,%f)\n"
			,xs[i] ,ys[i] ,zs[i], X[3*i] ,X[3*i+1] ,X[3*i+2]);
	}
*/	

#ifdef PRUNED_OUTLIER_CONTOURS 
	if(good_contour) {
#else
        if(1) {
#endif
   	   if(named) fprintf (fp,"%s ",name);
	   j = 0;
	   for (i=0;i<no;i++) {
		fprintf (fp,"%.3f %.3f  ",X[j], X[j+1]);
		j += 3;
	   }
	   fprintf (fp,"\n");
	} else {
	   fprintf(stderr,"Contour outside Ybase zone or not good\n");
        }
}

void init_output_featxy (FILE *fp_out, upsegQueryInfo *info
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

double yfromi(int i, Yfont *yfont)
{
   double y;
   
   y = yfont->ymin + (double) i * (yfont->ymax - yfont->ymin)
         /(double) yfont->nbins;
   return(y);      
}

void init_yfont_parameters(Yfont *yfont, int nentries)
{
   yfont->base = 0.0;
   yfont->mean = 0.0;
   yfont->n = 0;
   yfont->sd = 0.0;
   yfont->sdb = 0.0;
   yfont->ndb = 0;
   yfont->ymin = 1.e60;
   yfont->ymax = -1.e60;
   yfont->nbins = 0;
   yfont->yhist = NULL;
   yfont->ncont = nentries;
}

void accumulate_yfont_parameters(Yfont *yfont,int xi[],int yi[],int zi[],int ns)
{
   int i;
   
   for(i = 0; i < ns; ++i) {
      if(zi[i] >= PRS_THRESHOLD) {
         yfont->mean += yi[i];
         yfont->sd += yi[i] * yi[i];
         yfont->n += 1;
         if(yi[i] < yfont->ymin) {
            yfont->ymin = yi[i];
         }
         if(yi[i] > yfont->ymax) {
            yfont->ymax = yi[i];
         }
      }
   }
}

void yfont_histogram_parameters(Yfont *yfont,int xi[],int yi[],int zi[],int ns)
{
   int i,ibin;
   double y, yrange;
   
   yrange = yfont->ymax - yfont->ymin + 1;
   if(yrange < 1.) yrange = 1.;
   
   for(i = 0; i < ns; ++i) {
      if(zi[i] >= PRS_THRESHOLD) {
         y = (double) yfont->nbins * (yi[i] - yfont->ymin) / yrange;
         ibin = (int) y;
         if(ibin < 0) ibin = 0;
         if(ibin >= yfont->nbins) {
            ibin = yfont->nbins - 1;
         }
         yfont->yhist[ibin] += 1;
      }
   }
}

void boundingbox(int xi[],int yi[],int zi[],int ns, Box *box)
{
   int i,nu;
   
   box->xmin = box->ymin = 1.e60;
   box->xmax = box->ymax = -1.e60;
   box->xmean = box->ymean = 0.0;
   nu = 0;
   for(i = 0; i < ns; ++i) {
      if(zi[i] >= PRS_THRESHOLD) {
         if(xi[i] < box->xmin) box->xmin = xi[i];
         if(yi[i] < box->ymin) box->ymin = yi[i];
         if(xi[i] > box->xmax) {
            box->xmax = xi[i];
            box->irightmost = i;
         }
         if(yi[i] > box->ymax) box->ymax = yi[i];
         
         box->xmean += xi[i];
         box->ymean += yi[i];
         ++nu;
      }
   }
   if(nu < 1) nu = 1;
   box->xmean /= nu;
   box->ymean /= nu;
}

void yfont_sdy_of_base_boxes(Yfont *yfont,int xi[],int yi[],int zi[],int ns)
{
   Box box;
   
   boundingbox(xi,yi,zi,ns,&box);
   
   if(yfont->base >= box.ymin &&
      yfont->base <= box.ymax) {
         yfont->sdb += box.ymean * box.ymean;
         yfont->ndb += 1;
   }
}

void yfont_histogram_filter(Yfont *yfont)
{
   int i;
   double *yhist_filt;
   
   yhist_filt = (double *) calloc(yfont->nbins,sizeof(double));
   if(yhist_filt == NULL) {
      fprintf(stderr,"Error malloc yfont yhist[]\n");
      exit(1);
   }
      
   for(i = 1; i < yfont->nbins-1; ++i) {
      yhist_filt[i] = yfont->yhist[i-1]/3. 
                    + yfont->yhist[i]/3. 
                    + yfont->yhist[i+1]/3.;      
   }
   yhist_filt[0] = 0.0;
   yhist_filt[yfont->nbins-1] = 0.0;
   
   for(i = 0; i < yfont->nbins; ++i) {
      yfont->yhist[i] = yhist_filt[i];
   }
   free(yhist_filt);
}

void finish_yfont_parameters(Yfont *yfont)
{
   
   if(yfont->n < 1) yfont->n = 1;
   yfont->mean = yfont->mean / yfont->n;

   yfont->sd = yfont->sd / yfont->n - yfont->mean * yfont->mean;
   if(yfont->sd < 0.0) yfont->sd = 0.0;
   yfont->sd = sqrt(yfont->sd);

   yfont->nbins = yfont->ymax - yfont->ymin + 1;
   if(yfont->nbins > 1000) {
      yfont->nbins = 1000;
   }
   yfont->yhist = (double *) calloc(yfont->nbins,sizeof(double));

   /* note: alloc nbins peaks: since the test for peak presence
      involves a range of 5 samples, the number of peaks will never
      be larger than nbins/5. So: nbins peaks is always enough */
      
   yfont->ypeaklist = (double *) calloc(yfont->nbins,sizeof(double));
   yfont->ipeakpos = (int *) calloc(yfont->nbins,sizeof(int));
   yfont->npeaks = 0;
}

void finish_yfont_sdy(Yfont *yfont)
{
   double r;
   
   if(yfont->ndb < 1) yfont->ndb = 1;
   r = yfont->sdb / yfont->ndb - yfont->base * yfont->base;
   if(r < 0.0) r = 0.0;
   yfont->sdb = sqrt(r);
}

void free_yfont_parameters(Yfont *yfont)	
{
   if(yfont->yhist != NULL) {
      free(yfont->yhist);
   }
   if(yfont->ypeaklist != NULL) {
      free(yfont->ypeaklist);
   }
   if(yfont->ipeakpos != NULL) {
      free(yfont->ipeakpos);
   }
}

void yfont_peaklist(Yfont *yfont)	
{
   int i,ipeak,ilargest,prevpeak;
   int jump = 2; /* prevent silly peaks only 1 or 2 bins apart */
   double pmax;
   
   ipeak = 0;
   ilargest = 0;
   pmax = -1.;
   prevpeak = -9999;
   for(i = 2; i < yfont->nbins-2; ++i) {
      if(yfont->yhist[i] > yfont->yhist[i-2] 
      && yfont->yhist[i] > yfont->yhist[i+2]
      && i > prevpeak + jump
      ) {
         yfont->ypeaklist[ipeak] = yfromi(i,yfont); 
         yfont->ipeakpos[ipeak] = i;
         if(yfont->yhist[i] > pmax) {
            pmax = yfont->yhist[i];
            yfont->base = yfont->ypeaklist[ipeak];
         }      
         prevpeak = i;          
         ipeak++;
      }
   }
   yfont->npeaks = ipeak;
   if(pmax == -1.) {
      yfont->base = yfont->mean;
   }
}

void yfont_sort_peaklist(Yfont *yfont)	
{
   int i,j,*idx,*ipos;
   double *p;
  
   idx = (int *) malloc(yfont->npeaks * sizeof(int));
   if(idx == NULL) {
      fprintf(stderr,"No peaks to malloc?\n");
      return;
   }
   ipos = (int *) malloc(yfont->npeaks * sizeof(int));
   if(ipos == NULL) {
      fprintf(stderr,"No peaks to malloc?\n");
      return;
   }
   p = (double *) malloc(yfont->npeaks * sizeof(double));
   if(p == NULL) {
      fprintf(stderr,"No peaks to malloc?\n");
      return;
   }
   for(i = 0; i < yfont->npeaks; ++i) {
      p[i] = yfont->yhist[yfont->ipeakpos[i]];
   }
   
   shell_sortd(p,yfont->npeaks,idx,-1);
   
   for(i = 0; i < yfont->npeaks; ++i) {
      p[i] = yfont->ypeaklist[i];
      ipos[i] = yfont->ipeakpos[i];
   }
   
   for(i = 0; i < yfont->npeaks; ++i) {
      j = idx[i];
      yfont->ypeaklist[i] = p[j];
      yfont->ipeakpos[i] = ipos[j];
   }
   
   free(ipos);
   free(idx);
   free(p);
}

void yfont_journal(Yfont *yfont, int npeaks_max)
{
   FILE *fp;
   int i,n;
   
   fp = fopen("yfont.par","w");
   if(fp == NULL) {
      fprintf(stderr,"yfont_journal() Error opening for output\n");
      return;
   }
   
   fprintf(fp,"ymin=  %f\n", yfont->ymin);
   fprintf(fp,"ymean= %f\n", yfont->mean);
   fprintf(fp,"ybase= %f\n", yfont->base);
   fprintf(fp,"ymax=  %f\n", yfont->ymax);
   fprintf(fp,"ysd=   %f\n", yfont->sd);
   fprintf(fp,"ysdb=   %f\n", yfont->sdb);
   fprintf(fp,"Npoints= %d\n", yfont->n);
   fprintf(fp,"Ninbase= %d\n", yfont->ndb);
   fprintf(fp,"Ncontou= %d\n", yfont->ncont);
   fprintf(fp,"Nbins=   %d\n", yfont->nbins);
   fprintf(fp,"Npeaks=   %d\n", yfont->npeaks);
   fclose(fp);


   fp = fopen("yfont.peaks","w");
   if(fp == NULL) {
      fprintf(stderr,"yfont_journal() Error opening for output\n");
      return;
   }
   fprintf(stderr,"npeaks=%d, writing largest %d\n",yfont->npeaks,npeaks_max);
   n = npeaks_max;
   if(n > yfont->npeaks) n = yfont->npeaks;
   for(i = 0; i < n; ++i) {
      fprintf(fp,"%f %f\n"
                          , yfont->ypeaklist[i]
                          , yfont->yhist[yfont->ipeakpos[i]]
                          );
   }
   fclose(fp);

   fp = fopen("yfont.hist","w");
   if(fp == NULL) {
      fprintf(stderr,"yfont_journal() Error opening for output\n");
      return;
   }
   
   for(i = 0; i < yfont->nbins; ++i) {
      fprintf(fp,"%f %f\n"
                , yfromi(i,yfont)
                , yfont->yhist[i]);
   }
   fclose(fp);
}

void foreach_entry(tUPUnipen *pUnipen
                     ,tUPEntry **entries
                     ,int nentries
                     ,sigCharStream **streams
                     ,Yfont *yfont
                     ,void (*operator)(Yfont *,int *,int *,int *,int))
{
   tUPEntry *entry;
   int i,ns, *xi,*yi,*zi, ns_lower;
   Box box;
   
   for (i=0;i<nentries;i++) {
	entry = entries[i];
	fprintf(stderr,".");
		
	sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
        boundingbox(xi,yi,zi,ns,&box);
        
        /* Only lower contour: */
        
        ns_lower = box.irightmost;
        
        operator(yfont,xi,yi,zi,ns_lower);
	free(xi);
        free(yi);
	free(zi);
   }
   fprintf(stderr,"\n");
}

void yfont_estimation(tUPUnipen *pUnipen
                     ,tUPEntry **entries
                     ,int nentries
                     ,sigCharStream **streams
                     ,Yfont *yfont
                     ,int npeaks_max)
{
   int ifilt,nfilt = 51;

   init_yfont_parameters(yfont,nentries);

   fprintf(stderr,"yfont step1: range, mean and sd of Y\n");

   foreach_entry(pUnipen,entries,nentries,streams,yfont
                                      ,accumulate_yfont_parameters);

   finish_yfont_parameters(yfont);

   fprintf(stderr,"yfont step2: histogram of Y\n");

   foreach_entry(pUnipen,entries,nentries,streams,yfont
                                      ,yfont_histogram_parameters);
   
   fprintf(stderr,"yfont step3: filter Yhistogram\n");
   
   for(ifilt = 0; ifilt < nfilt; ++ifilt) {
      yfont_histogram_filter(yfont);
   }
   
   fprintf(stderr,"yfont step4: peak determination of Yhistogram\n");

   yfont_peaklist(yfont);
   
   yfont_sort_peaklist(yfont);
   
   if(nentries == 1) { /* No use in relying on the (peak) shape of the
                          histogram in case of only a single contour */
                          
      yfont->base = yfont->mean;
   }
   
   foreach_entry(pUnipen,entries,nentries,streams,yfont
                                      ,yfont_sdy_of_base_boxes);
   finish_yfont_sdy(yfont);                                      

   yfont_journal(yfont,npeaks_max);
}

int output_featxy (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams
	, int named
	, Yfont *yfont)
{
	tUPEntry *entry,**segment_entries;
	int i,ns, *xi,*yi,*zi;
	char name[2000];
	char segname[2000];
	char dum[2000];

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	
        if(oinfo->norm_pos_size == NORM_MID_X_BASELINE_Y) {
           yfont_estimation(pUnipen,entries,nentries,streams,yfont,NPEAKS_MAX);
        } else {
           yfont->base = 0.0;
        }
        
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		sscanf(entry->Entry,"%s%s%s%s%s",dum,dum,dum,dum,segname);
		clean_segment_name(segname);
		
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		sprintf(name,"%s/%s",oinfo->writer_codestr,segname);
		fprintf (stderr,"outputing %d (%d,%d,%d) (%d,%d,%d)\n"
			,ns,xi[0],yi[0],zi[0],xi[ns-1],yi[ns-1],zi[ns-1]);
		output_contour_xy(fp_out,name,ns,oinfo->mreq_nsamples,xi,yi,zi
		                        , named, oinfo->norm_pos_size,yfont);
		free(xi);
		free(yi);
		free(zi);
	}
        if(oinfo->norm_pos_size == NORM_MID_X_BASELINE_Y) {
           free_yfont_parameters(yfont);	
        }
	return 1;
}
