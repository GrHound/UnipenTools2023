/* Program to compute statistics or features from a UNIPEN
   file with on-line handwriting or contour vectors of off-line
   handwriting */

#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>

#include "allo_resample.h"
#define _MAIN
#include "upstat1.h"

#define CLSWID_X_PIX 10       
#define CLSWID_Y_PIX 2       
#define YBINS  12000  /* for 300dpi this would be 40 inches 
                         and thus sufficient for common documents */
                         
#define RIDICULOUSLY_BIG 9999999.                        

typedef struct {
   double xmin;
   double xmax;
   double ymin;
   double ymax;
   double wmin;
   double wmax;
   double hmin;
   double hmax;
   double ybaseline;
   double pmax;
} Parms;                         
                        

int extremum(float arr[], int i, int n) 
{
   int iret;
   int pixjump = 2;
   
   iret = 0;
   
   if(i >= pixjump && i < (n - pixjump)) {
      if(arr[i-pixjump] < arr[i] && arr[i] > arr[i+pixjump]) {
         iret = 1;  /* maximum at i */
      }
      if(arr[i-pixjump] > arr[i] && arr[i] < arr[i+pixjump]) {
         iret = 1;  /* minimum at i */
      }
   }
   return(iret);
}

int maximum(float arr[], int i, int n) 
{
   int iret;
   int pixjump = 3;
   
   iret = 0;
   
   if(i >= pixjump && i < (n - pixjump)) {
      if(arr[i-pixjump] < arr[i] && arr[i] > arr[i+pixjump]) {
         iret = 1;  /* maximum at i */
      }
   }
   return(iret);
}

int minimum(float arr[], int i, int n) 
{
   int iret;
   int pixjump = 3;
   
   iret = 0;
   
   if(i >= pixjump && i < (n - pixjump)) {
      if(arr[i-pixjump] > arr[i] && arr[i] < arr[i+pixjump]) {
         iret = 1;  /* minimum at i */
      }
   }
   return(iret);
}

void movavg2(double rawin[], double fltout[], int n)
{
   int i,j,k;

   for(i = -1; i < n; ++i) {
      j = i;
      if(j < 0) j = n - 1;
      k = i + 1;
      if(k > n - 1) k = 0;
      
      fltout[k] = (rawin[j]+rawin[k])/2.;
   }
}

void nsmooth2(double arr[], int n, int nsmo)
{ /* binomial impulse response through iterative 2-sample box-car 
     convolution */

   int k;
   double *tmparr;

   if(nsmo > 0) {
       tmparr = malloc(n * sizeof(double));
       if(tmparr == NULL) {
         fprintf(stderr,"Error malloc smooth() tmparr[]\n");
         exit(1);
       }
       for(k = 0; k < nsmo; ++k) {
         movavg2(arr,tmparr,n);
         movavg2(tmparr,arr,n);
       }
       free(tmparr);
   }
}

void collect_unipen_features (FILE *fp, char *name, int ns
         , int m, int *xi, int *yi, int *zi
         , int named, int norm_pos_size
         , double f_pdf[], int nbins, int ifeature
         , Parms *par, XY_Chunk xyobj[], int nobj)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	int offset, i, k, nextr, nmin, nmax;
	double xmin, xmax, ymin, ymax, xavg, yavg;
	double w, h;
	int iobj;

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
	
 /* find bounding box and c.o.g. of this contour */

	xmin = xmax = xi[offset];
	ymin = ymax = yi[offset];
	xavg = yavg = 0.0;
	
	for (i=0;i<ns;i++) {
		x[i] = (float) xi[i+offset];
		y[i] = (float) yi[i+offset];
		z[i] = (float) zi[i+offset];
		 
		xavg += x[i];
		yavg += y[i];
		
	        if(x[i] < xmin) xmin = x[i];
	        if(x[i] > xmax) xmax = x[i];
	        if(y[i] < ymin) ymin = y[i];
	        if(y[i] > ymax) ymax = y[i];
	}
	
        w = xmax - xmin;        
        h = ymax - ymin;
        if(ns > 0) {
           xavg = xavg / ns;
           yavg = yavg / ns;
        }
        
 /* count special values */
	        
	nextr = nmin = nmax = 0;

	for (i=0;i<ns;i++) {
#ifdef VERBOSE	        
	        fflush(stderr);
	        printf("x=%f y=%f z=%f\n",x[i],y[i],z[i]);
	        fflush(stdout);
#endif
		
	        if(ifeature == _OUTPUT_XPDF) {
	           k = x[i]/CLSWID_X_PIX;   /* geen linkermarge offset */
	           if(k < 0) k = 0;
	           if(k >= nbins) k = nbins - 1;
	           f_pdf[k] += 1.;
	        } else if(ifeature == _OUTPUT_YPDF) {
	          
#ifdef FUSED
	           if(extremum(y,i,ns)) {
	              k = nbins/2 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	              if(k < 0) k = 0;
	              if(k >= nbins) k = nbins - 1;
	              f_pdf[k] += 1.;
	              ++nextr;
	           }
	           nmin = nmax = -1;
#endif

#ifdef MINMAX
                   /* apart lower en upper */
                   if(i < ns/2) {
	              if(minimum(y,i,ns)) {
	                 k = nbins/4 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	                 if(k < 0) k = 0;
	                 if(k >= nbins) k = nbins - 1;
	                 f_pdf[k] += 1.;
	                 ++nextr;
	                 ++nmin;
	              } 
	           } else {
	              if(maximum(y,i,ns)) {
	                 k = nbins/2 + nbins/4 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	                 if(k < 0) k = 0;
	                 if(k >= nbins) k = nbins - 1;
	                 f_pdf[k] += 1.;
	                 ++nextr;
	                 ++nmax;
	              }
	           }
	           
#endif

#ifdef LEFTRIGHT
                   /* apart left & right  */
                   if(x[i] < (par->xmin+par->xmax)/2.) {
	              if(extremum(y,i,ns)) {
	                 k = nbins/4 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	                 if(k < 0) k = 0;
	                 if(k >= nbins) k = nbins - 1;
	                 f_pdf[k] += 1.;
	                 ++nextr;
	                 ++nmin;
	              } 
	           } else {
	              if(extremum(y,i,ns)) {
	                 k = nbins/2 + nbins/4 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	                 if(k < 0) k = 0;
	                 if(k >= nbins) k = nbins - 1;
	                 f_pdf[k] += 1.;
	                 ++nextr;
	                 ++nmax;
	              }
	           }
	           
#endif

#define BIGSMALL
#ifdef BIGSMALL
                   /* apart small & big  */
                   if(w < (par->wmin+par->wmax)/2.) {
	              if(extremum(y,i,ns)) {
	                 k = nbins/4 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	                 if(k < 0) k = 0;
	                 if(k >= nbins) k = nbins - 1;
	                 f_pdf[k] += 1.;
	                 ++nextr;
	                 ++nmin;
	              } 
	           } else {
	              if(extremum(y,i,ns)) {
	                 k = nbins/2 + nbins/4 - (y[i] - par->ybaseline)/CLSWID_Y_PIX;   
	                 if(k < 0) k = 0;
	                 if(k >= nbins) k = nbins - 1;
	                 f_pdf[k] += 1.;
	                 ++nextr;
	                 ++nmax;
	              }
	           }
	           
#endif

	        }
	}
	
        printf("xmin=%f xmax=%f\n",xmin,xmax);
        printf("ymin=%f ymax=%f\n",ymin,ymax);
        printf("Nextrema=%d in Nsamples=%d Nmin=%d Nmax=%d\n",nextr,ns,nmin,nmax);
        fflush(stdout);
        
        iobj = nobj-1; /* last chunk */
        xyobj[iobj].xmin = xmin;
        xyobj[iobj].ymin = ymin;
        xyobj[iobj].xmax = xmax;
        xyobj[iobj].ymax = ymax;
        xyobj[iobj].width = xmax-xmin;
        xyobj[iobj].height = ymax-ymin;
        xyobj[iobj].ybase = par->ybaseline;
	
        if(ifeature == _OUTPUT_XPDF) {
                   /* already filled */
                   
        } else if(ifeature == _OUTPUT_YPDF) {
                   /* already filled */

        } else if(ifeature == _OUTPUT_WPDF) {
	           k = w/5;   
	           if(k < 0) k = 0;
	           if(k >= nbins) k = nbins - 1;
	           f_pdf[k] += 1.;
	           
        } else if(ifeature == _OUTPUT_HPDF) {
	           k = h;   
	           if(k < 0) k = 0;
	           if(k >= nbins) k = nbins - 1;
	           f_pdf[k] += 1.;
	           
	} else {
	  fprintf(stderr,"Error output_function (feature) %d not impl.\n",ifeature);
	  exit(1);
	}
}

void init_output_xpdf (FILE *fp_out, upsegQueryInfo *info
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

void clean_xpdf_segment_name(char *segname)
{
   int last;
   last = strlen(segname)-1;
   if(last > 0) {
      if(segname[0] == '"') segname[0] = '-';
      if(segname[last] == '"') segname[last] = ' ';
   }
}

void layout_unipen_contours (upsegQueryInfo *info, OFunction_Info *oinfo
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams
	, int named, Parms *par)
{
	tUPEntry *entry,**segment_entries;
	int i,j,k,ns, *xi,*yi,*zi, nbins_y;
	char segname[2000];
	char dum[2000];
	double *ypdf, w, h;
	Parms this;
	
        nbins_y = YBINS;
	
        ypdf = (double *) calloc(nbins_y, sizeof(double));
        if(ypdf == NULL) {
           fprintf(stderr,"Error calloc ypdf[]\n");
           exit(1);
        }
        
	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	par->xmin = par->ymin = par->wmin = par->hmin = RIDICULOUSLY_BIG; 
	par->xmax = par->ymax = par->wmax = par->hmax = -RIDICULOUSLY_BIG;
	
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		sscanf(entry->Entry,"%s%s%s%s%s",dum,dum,dum,dum,segname);
		clean_xpdf_segment_name(segname);
		
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);

	        this.xmin = this.ymin = RIDICULOUSLY_BIG;
	        this.xmax = this.ymax = -RIDICULOUSLY_BIG;

		for(j = 0; j < ns; ++j) {
		  k = yi[j];
		  if(k < 0) {
		     k = 0;
		     fprintf(stderr,"Error: negative y coordinate y=%d\n", yi[j]);
		  }
		  if(k >= nbins_y) {
		     fprintf(stderr,"Error: y coordinate overflow: y=%d\n", yi[j]);
		     k = oinfo->nbins - 1;
		  }

		  ypdf[k] += 1.0;
		  
		  /* for set of contours: */

		  if(xi[j] < par->xmin) par->xmin = xi[j];
		  if(xi[j] > par->xmax) par->xmax = xi[j];

		  if(yi[j] < par->ymin) par->ymin = yi[j];
		  if(yi[j] > par->ymax) par->ymax = yi[j];
		  
		  /* current contour: */

		  if(xi[j] < this.xmin) this.xmin = xi[j];
		  if(xi[j] > this.xmax) this.xmax = xi[j];

		  if(yi[j] < this.ymin) this.ymin = yi[j];
		  if(yi[j] > this.ymax) this.ymax = yi[j];
	        }

	        w = this.xmax - this.xmin;
	        printf("this.w=%f\n",w);
	        h = this.ymax - this.ymin;
	        
              if(i < nentries -1) {	        
	        if(w < par->wmin) par->wmin = w;
	        if(h < par->hmin) par->hmin = h;

	        if(w > par->wmax) par->wmax = w;
	        if(h > par->hmax) par->hmax = h;
	      }

		free(xi);
		free(yi);
		free(zi);
	}
	nsmooth2(ypdf, nbins_y, 3);
	
        	
	fprintf(stdout,"%s ", oinfo->tag);
	par->pmax = 0.0;
	par->ybaseline = (par->ymin+par->ymax)/2.;
	
	for(i = 0; i < nbins_y; ++i) {
#ifdef VERBOSE
	   fprintf(stdout,"%d %f\n", i, ypdf[i]);
#endif
	   
	   if(ypdf[i] > par->pmax) {
	      par->pmax = ypdf[i];
	      par->ybaseline = i;
	   }
        }
         
       
        fprintf(stdout,"\n");

        printf("ymin=%f ymax=%f ybaseline=%f pmax=%f\n"
              ,par->ymin,par->ymax,par->ybaseline,par->pmax);
        fflush(stdout);

       
        free(ypdf);
}

int output_xpdf (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams
	, int named
	, XY_Chunk **xyobj
	, int *nobj)
{
	tUPEntry *entry,**segment_entries;
	int i,ns, *xi,*yi,*zi;
	char name[2000];
	char segname[2000];
	char dum[2000];
	double *f_pdf, psum;
	Parms par;
	
        f_pdf = (double *) calloc(oinfo->nbins,sizeof(double));
        if(f_pdf == NULL) {
           fprintf(stderr,"Error calloc f_pdf[]\n");
           exit(1);
        }
        
        layout_unipen_contours(info,oinfo
	                         , pUnipen, entries, nentries
	                         , level_names, names, streams
	                         , named, &par);

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		sscanf(entry->Entry,"%s%s%s%s%s",dum,dum,dum,dum,segname);
		clean_xpdf_segment_name(segname);
		
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		sprintf(name,"%s/%s",oinfo->writer_codestr,segname);
		fprintf (stderr,"outputing %d (%d,%d,%d) (%d,%d,%d)\n"
			,ns,xi[0],yi[0],zi[0],xi[ns-1],yi[ns-1],zi[ns-1]);
		collect_unipen_features(fp_out,name,ns,oinfo->m,xi,yi,zi
		                   , named, oinfo->norm_pos_size
		                   , f_pdf, oinfo->nbins
		                   , oinfo->output_function
		                   , &par, *xyobj, *nobj);

                *nobj += 1;
                *xyobj = (XY_Chunk *) realloc(*xyobj,*nobj * sizeof(XY_Chunk));
                if(*xyobj == NULL) {
                   fprintf(stderr,"Error realloc xyobj[]\n");
                   exit(1);
                }
               
		free(xi);
		free(yi);
		free(zi);
	}
	
/* smooth */

        nsmooth2(f_pdf,oinfo->nbins,oinfo->nsmo);

/* normalize histogram to area of 1.0 to obtain a PDF */

        psum = 0.0;        
	for(i = 0; i < oinfo->nbins; ++i) {
	   psum += f_pdf[i];
        }
        if(psum == 0.0) psum = 1.0;
	for(i = 0; i < oinfo->nbins; ++i) {
	   f_pdf[i] /= psum;
        }       

/* Write the vector out, together with its label in the first column */ 
      
	fprintf(fp_out,"%s ", oinfo->tag);
	for(i = 0; i < oinfo->nbins; ++i) {
	   fprintf(fp_out,"%f ", f_pdf[i]);
        }
        fprintf(fp_out,"\n");
        printf("(written %d feature values of type %s)\n"
                , oinfo->nbins
                , output_function_names[oinfo->output_function]);
                
/* clean up */               
        free(f_pdf);

	return 1;
}

