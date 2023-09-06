#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uplib.h>
#include <upsiglib.h>

/*
#include <vhs.h>
#include <vhs_proto.h>
#include <vhs_hclus.h>
*/

#include "output_featxy.h"
#include "writer_identifications.h"
#include "upread3.h"
#include "output_image.h"
#include "image_routines.h"
#include "allo_resample.h"

IMG read_canvas(OFunction_Info *oinfo)
{
  IMG image;
  double Gmax;
  
  fprintf(stderr,"(reading canvas image file [%s])\n"
                ,oinfo->canvas);
                
  read_data (oinfo->canvas, &image
            , &oinfo->width, &oinfo->height, &Gmax);
  return(image);
}

double radius_normalized_to_pixel_scale(float X[], float Y[]
                                          , int ns, int w, int h, int margin
                                          , double *half)
{
   int i;
   double s,xmin,xmax,ymin,ymax,xrange,yrange;
   double ww, hh;
   
   ww = (double) w - 2. * margin;
   if(ww < 3) {
      fprintf(stderr,"margin=%d too big for image width=%d\n",margin,w);
      exit(1);
   }
   
   hh = (double) h - 2. * margin;
   if(hh < 3) {
      fprintf(stderr,"margin=%d too big for image height=%d\n",margin,h);
      exit(1);
   }
   
   xmin = xmax = X[0];
   ymin = ymax = Y[0];
   for(i = 0; i < ns; ++i) {
      if(X[i] < xmin) xmin = X[i];
      if(Y[i] < ymin) ymin = Y[i];
      if(X[i] > xmax) xmax = X[i];
      if(Y[i] > ymax) ymax = Y[i];
   }
   xrange = xmax - xmin + 1.;
   yrange = ymax - ymin + 1.;
   
   if(xrange > yrange) {
      *half = xrange / 2.;
   } else {
      *half = yrange / 2.;
   }
   
   if(ww > hh) { 
      if(yrange/h > xrange/w) {
         s = hh / 2.;
      } else {
         s = ww / 2.;
      }
   } else {
      if(xrange/w > yrange/h) {
         s = ww / 2.;
      } else {
         s = hh / 2.;
      }
   }
   return(s);
}

void float_mean_xy(float x[], float y[], int ns, double *xm, double *ym)
{
  int i;
  
  *xm = 0.0;
  *ym = 0.0;
  if(ns < 1) {
    fprintf(stderr,"float_mean_xy() No samples?\n");
    return;
  }
  for (i=0; i < ns; i++) {
     *xm += (double) x[i];
     *ym += (double) y[i];
  }
  *xm = *xm / ns;
  *ym = *ym / ns;
}

void float_scale_to_pixels(float x[], float y[], float z[], int ns
                          ,float xs[], float ys[], float zs[], float va[]
                          ,int *no
                          ,int req_scalemode
                          ,int w, int h, char canvasfile[]
                          ,int margin, double range
                          ,int mreq_nsamples
                          ,double dfactor)
{        
   float X[NSMAX],Y[NSMAX],Z[NSMAX];
   double xm, ym, scalefact, half;
   int i, scalemode;

   if(req_scalemode == NORM_RAW) {
      scalemode = 0;
      fprintf(stderr,"Requested -scale: rawxy\n");
                
   } else if(canvasfile[0] == NUL) {
      fprintf(stderr,"Requested -scale %s: but No canvas, setting scalemode=0\n", scalenames[req_scalemode]);
      scalemode = 0;
   } else {
      fprintf(stderr,"Requested -scale %s: setting image scalemode=1\n", scalenames[req_scalemode]);
      scalemode = 1;
   }

   if (req_scalemode == NORM_ALLO_POS_RADIUS) {
       fprintf(stderr,"Taking normalized and resampled (m=%d) coordinates\n", mreq_nsamples);
       *no = mreq_nsamples;
       recog_spatial_z_sampler(x,y,z,ns
	       ,xs,ys,zs,*no,va,0,ns-1,15.);
       if (dfactor!=-1.0) {
	       adjust_resampling (ns,x,y,z,*no,xs,ys,zs,dfactor);
       }
       float_normalize_allo_xy(*no,xs,ys,X,Y);
       scalefact = radius_normalized_to_pixel_scale(X,Y,ns,w,h,margin,&half);
       for (i=0; i < *no; i++) {
	       xs[i] = (double) w/2. + scalefact * X[i];
	       ys[i] = (double) h/2. - scalefact * Y[i];
	       zs[i] = (double) z[i];
       }		

   } else if (req_scalemode == NORM_EXPAND) {
       fprintf(stderr,"Taking expanded coordinates (first bump x or y)\n");

       scalefact = radius_normalized_to_pixel_scale(x,y,ns,w,h,margin,&half);

       *no = ns;
       
       float_mean_xy(x,y,ns,&xm,&ym);
       
       for (i=0; i < *no; i++) {
	       xs[i] = (double) w/2. + scalefact * (x[i]-xm)/half;
	       ys[i] = (double) h/2. - scalefact * (y[i]-ym)/half;
	       zs[i] = (double) z[i];
       }		

   } else if (req_scalemode == NORM_MIDDLE) {

       fprintf(stderr,"Taking raw Unipen coordinates for -scale %s, shift x,y to middle of image\n",scalenames[req_scalemode]);
       *no = ns;
       float_mean_xy(x,y,ns,&xm,&ym);

       for (i=0; i < *no; i++) {
	       xs[i] = (double) w/2. + 0.9 * (x[i]-xm);
	       ys[i] = (double) h/2. - 0.9 * (y[i]-ym);
	       zs[i] = (double) z[i];
       }		

   } else if (req_scalemode == NORM_LEFT) {

       fprintf(stderr,"Taking raw Unipen coordinates for -scale %s, shift x,y to middle of image\n",scalenames[req_scalemode]);
       *no = ns;
       xm = 99999999.;
       ym = 0.0;
       for (i=0; i < *no; i++) {
               if(x[i] < xm) {
	          xm = x[i];
	       }
	       ym += (double) y[i];
       }
       ym = ym / *no;

       for (i=0; i < *no; i++) {
	       xs[i] = margin        + 0.9 * (x[i]-xm);
	       ys[i] = (double) h/2. - 0.9 * (y[i]-ym);
	       zs[i] = (double) z[i];
       }		

   } else {

       fprintf(stderr,"Taking raw Unipen coordinates for scalemode=%d\n",scalemode);
       *no = ns;
       for (i=0; i < *no; i++) {
	       X[i] = (double) x[i];
	       Y[i] = (double) y[i];
	       Z[i] = (double) z[i];
	       zs[i] = z[i];
       }
       float_scale_allo(*no,X,Y,Z,xs,ys,zs,w,h,margin,range,scalemode);

   }
}

void do_output_image (char *fname, IMG image, int brush, int margin
	, int w, int h
	, int ns, int mreq_nsamples, int *xi, int *yi, int *zi
	, int ofrmt, double dfactor, double range
	, int req_scalemode, char canvasfile[]
	, int filled, int nsmo, int inkmodel)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	int offset, i, no, ns_org, col;
	static int rancol = -5;
	
        if(ofrmt != O_PPM) {
            col = GREY_PIXEL;
            fprintf(stderr,"-frmt is not ppm: assume grey ink\n");
        } else {
            fprintf(stderr,"-frmt is ppm: assume random ink color\n");
            col = rancol;
        }


	ns_org = ns;
	/* remove pen_up head */
	
	offset = 0;
	while (zi[offset]<15.) {
		if (offset >= ns-1) {
			fprintf (stderr,"skipping penup segment!\n");
			return;
		}
		offset++;
	}
	/* remove pen-up tail */
	ns = ns - 1;
	while (zi[ns]<15.&&ns>offset) {
		ns--;
	}

	ns = ns-offset+1;
	for (i=0;i<ns;i++) {
		x[i] = (float) xi[i+offset];
		y[i] = (float) yi[i+offset];
		z[i] = (float) zi[i+offset];
	}
	
        if(req_scalemode == NORM_ALLO_POS_RADIUS) {
           if(mreq_nsamples <= 1) {
              fprintf(stderr,"-scale normalize implies that -n <n.gt.1> is given. Current value: %d\n", mreq_nsamples);
              exit(1);
           }
        } else {
         
           if(mreq_nsamples > 1) {
              fprintf(stderr,"-n %d is ignored in -scale %s: no time axis normalized, please use -n -1 next time.\n"
                            , mreq_nsamples, scalenames[req_scalemode]);
           }
        }

	if(canvasfile[0] == NUL) {
  	     clear_image(image,w,h);
	}
	
        float_scale_to_pixels(x,y,z,ns,xs,ys,zs,va,&no
                                      ,req_scalemode,w,h,canvasfile
                                      ,margin,range
                                      ,mreq_nsamples,dfactor);

        float_plot_allo_in_image(image,w,h,no,xs,ys,zs
                                      ,col,brush,filled,nsmo,inkmodel);
		
	write_image(fname,image,w,h,ofrmt);

        rancol -= 1;
        if(rancol < -250) rancol = -5;

}

void init_output_image (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo)
{
}

char *cleanlab(char tag[])
{
   static char out[2000];
   int i;
   
   strcpy(out,tag);
   if(tag[0] == '"') {
      strcpy(out,&tag[1]);
      i = strlen(out) - 1;
      if(i >= 0) {
         out[i] = (char) 0;
      }
      for(i = 0; i < strlen(out); ++i) {
         if(strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvw.,-"
            ,out[i]) == NULL ) {
                out[i] = '_';
         }
      }
   }
   return(out);
}

void make_image_filename(char fname[],int idx, char tag[], OFunction_Info *oinfo)
{
 if(tag[0] == NUL && oinfo->labeled_outfilnam == 0) {
   if (oinfo->im_frmt==O_XBM)
	sprintf (fname,"%s_%04d.xbm",oinfo->outfile,idx);
   else if (oinfo->im_frmt==O_PPM)
 	sprintf (fname,"%s_%04d.ppm",oinfo->outfile,idx);
   else if (oinfo->im_frmt==O_PGM)
	sprintf (fname,"%s_%04d.pgm",oinfo->outfile,idx);
   else {
	fprintf (stderr,"output format must be ppm xbm or pgm!\n");
	exit(1);
   }
 } else {
   if (oinfo->im_frmt==O_XBM)
	sprintf (fname,"%s_%s_%04d.xbm",oinfo->outfile,cleanlab(tag),idx);
   else if (oinfo->im_frmt==O_PPM)
 	sprintf (fname,"%s_%s_%04d.ppm",oinfo->outfile,cleanlab(tag),idx);
   else if (oinfo->im_frmt==O_PGM)
	sprintf (fname,"%s_%s_%04d.pgm",oinfo->outfile,cleanlab(tag),idx);
   else {
	fprintf (stderr,"output format must be ppm xbm or pgm!\n");
	exit(1);
   }
 }
}

int output_image (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams)
{
	tUPEntry *entry,**segment_entries;
	sigSignal *usignal;
	
	int i,ns,width,height, *xi,*yi,*zi;
	static IMG image = NULL;
	static int idx = 0;
	char fname[256], tmp[256], dum[2000], segmaybe[2000],segname[2000];
	char *key_entry;
	int pp_mm, pp_inch, this_range;
	
	/* Norm=range=3400 over 600 pixels width, 200 pixels height,
	   brush = 5, when ppmm was 50 */

	if(oinfo->range <= 0) {
	   /* try to get X-resolution */
	   if ((key_entry=upGetArgument(pUnipen,".X_POINTS_PER_MM",0)) 
	      != NULL) {
	   	sscanf(key_entry,"%s %d", tmp,&pp_mm);
	   	this_range = pp_mm * 68;
		fprintf(stderr,"%s found, taking ppmm=%d computed range (-r ...) = %d\n"
		              , key_entry, pp_mm, this_range);
	   } else {
		if ((key_entry=upGetArgument(pUnipen,".X_POINTS_PER_INCH",0))
		   !=NULL) {
		   	sscanf(key_entry,"%s %d", tmp,&pp_inch);
		   	pp_mm = (int) (((double) pp_inch / 25.6) + 0.5);
	   		this_range = pp_mm * 68;
			fprintf(stderr,"%s found, taking ppmm=%d computed range (-r ...) = %d\n"
			              , key_entry, pp_mm, this_range);
			
		} else {
			pp_mm = 50;
	   		this_range = pp_mm * 68;
			fprintf(stderr,"No POINTS_PER (MM or INCH), taking ppmm=%d computed range (-r ...) = %d\n"
			              , pp_mm, this_range);
	 	}
	   }

	} else {
	   this_range = oinfo->range;
	}
	if(this_range < 100) {
		this_range = 3400;
		fprintf(stderr,"Warning: Too small computed range, setting to default = %d\n"
		              , this_range);
	}

        if(oinfo->canvas[0] == NUL) {
	   width = oinfo->width;
	   height = oinfo->height;
	
           fprintf(stderr,"Requested image sizes: Width=%d Height=%d\n"
                      , width, height);
                      
	   if (image==NULL) {
	      image = create_image(width,height);
	   }
	} else {
	   if(image == NULL) {
	      image = read_canvas(oinfo);
	   }
	   width = oinfo->width;
	   height = oinfo->height;
	   if(oinfo->clearcanvas) {
	      fprintf(stderr,"(-clearcanvas, overwriting background image %s)\n"
	                    , oinfo->canvas);
	      clear_image(image,width,height);
	   }
        }

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		segname[0] = (char) 0;
                sscanf(entry->Entry,"%s%s%s%s%s",dum,dum,dum,segmaybe,segname);
                if(segname[0] == (char) 0) {
                  strcpy(segname,segmaybe);
                }
                fprintf (stderr,"Label=%s tagmode=%d\n", segname, oinfo->labeled_outfilnam);
		
		make_image_filename(fname,idx,segname,oinfo);
		
	        ++idx;
		
#ifdef LOE_UGLY_HANDLING_OF_PENUPS
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
#else 		
                usignal = sigCharstream2Signal(pUnipen,entry
                                                 ,TIME_EQUI_DIST,streams[i]);
                xi = usignal->x;                                               
                yi = usignal->y;                                               
                zi = usignal->z;
                ns = usignal->nsamples;                                               
#endif
	
		fprintf (stderr,"got %d samples\n",ns);
		
		do_output_image (fname,image,oinfo->brush,oinfo->margin
			,width,height,ns,oinfo->mreq_nsamples,xi,yi,zi
			,oinfo->im_frmt,oinfo->dfact
			,this_range,oinfo->norm_pos_size
			,oinfo->canvas
			,oinfo->filledcontours
			,oinfo->nsmooth
			,oinfo->inkmodel);
			
		if(i >= nentries -1) {
		   make_image_filename(fname,-1,"whole",oinfo);
	           write_image(fname,image,oinfo->width
	                       ,oinfo->height,oinfo->im_frmt);
	        }
			
		free(xi);
		free(yi);
		free(zi);
	}
	return 1;

}
