
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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <values.h>

#include "output_featxy.h"
#define _ALLO_RESAMPLE_INIT_
#include "allo_resample.h"

void allo_linearize (x, y, z, ns, prsmin)
float *x; float *y; float *z; int ns; double prsmin;
{

/* ADDED JANUARY 1996 BY OME LOE:
   before re-sampling, linearize each PEN-UP trajectory between the last and first PEN-DOWN samples
	s(d0)    = (x0,y0)
	s(d1)    = (x1,y1)

	let:
		s_up  = pen-up signal
		n_up  = number of pen-up samples
		dx    = (x1-x0)/n_up
		dy    = (y1-y0)/n_up
	
	and linearize the signal to become:
		s'_up(i) = (x0+i*dx,y0+i*dy)
*/

	int nadjusted=0,i,j,d0,d1,n_up;
	double dx,dy;

	for (i=0;i<ns;i++) {
		if (z[i]<=prsmin) {
			n_up = 0;
			d0 = d1 = i;
			while (d1<ns) {
				if (z[d1]<=prsmin)
					d1++;
				else
					break;
			}
			/* ok, now d1 points to the last pen-up sample OR
			              points to the last sample indexed by (ns-1)
			*/
			if ((n_up=d1-d0)>1) { /* else nothing to do */
				if (d0!=0)
					d0--;
				dx = (double) (x[d1]-x[d0])/n_up;
				dy = (double) (y[d1]-y[d0])/n_up;
				for (j=d0+1,i=0;j<d1;i++,j++) {
					x[j] = x[d0] + i*dx;
					y[j] = y[d0] + i*dy;
					nadjusted++;
				}
			}
			i = d1+1;
		}
	}
}


float recog_allo_interpolate (
/*
     linear interpolation of value in float array
     at (floating) index rn
*/
float  r[],   /* Array in which to interpolate */
int    j,     /* where to place it */
int    n,     /* Number of values in array r[] */
double rn,    /* Index to array r as floating value. */
float dflt)  /* default value to return if rn outside window */
/* float Function result: Interpolated value r[rn] */
{
	double ret_val, d;
	int i1, i2;
      
/* test */ 
 
	ret_val = dflt;
	if(rn < 0.0 || rn > (double) (n-1) + 0.01) { 
		fprintf(stderr, "%%recog_interpolate (%d) (warn) rn outside [0,%d]: %f\n",j, n-1,rn);
		fflush(stderr);
	}
	if(rn <= 0) { 
		ret_val = r[0];
	} else if(rn >= n-1) {
		ret_val = r[n-1];
	} else {
		i1 = (int) rn;
		i2 = i1 + 1;
		d = rn - (double) i1;
		ret_val = (1.-d) * r[i1] + d * r[i2];
	}
	return(ret_val);
}


void recog_spatial_z_sampler (
float  x_in[NSMAX],   /* (input) movement x coordinate */
float  y_in[NSMAX],   /* (input) movement y coordinate */
float  z[NSMAX],      /* (input) movement z coordinate */
int    n,             /* Number of samples */
float  xo[NSMAX],     /* (output) spatially normalized movement x coordinate */
float  yo[NSMAX],     /* (output) spatially normalized movement y coordinate */
float  zo[NSMAX],     /* (output) spatially normalized movement z coordinate */
int    no,            /* Requested number of output samples */
float  va[NSMAX],     /* (output) absolute velocity (should be flat) */
int    ii, int jj,         /* Beginning and end of strokes */
double prsmin)
{
	int i, j;
	double dx, dy, dc, cc, ctot, tc, to;
	double dc_rest, a, b, cco;
	float x[NSMAX],y[NSMAX];

	memcpy(x,x_in,n*sizeof(float));
	memcpy(y,y_in,n*sizeof(float));

	allo_linearize(x,y,z,n,prsmin);
	
/*  Calculate the deltas va[i] and the total trajectory length CTOT 
    of the ink */ 
 	
	va[ii] = 0.0;
	ctot = 0.0;
	for(i=(ii+1); i <= jj; ++i) {
		dx=x[i]-x[i-1];
		dy=y[i]-y[i-1];
		va[i] = sqrt(dx*dx+dy*dy);
		ctot = ctot + va[i];
		if(i >= n) break;
	}
 
/*  Calculate the average delta dC */ 
 
	dc = ctot / (no-1);
	if(dc < 1.0E-10) {
		fprintf(stderr,"%%recog_spatial_sampler, zero trajectory length (ctot=%f)(ni=%d,no=%d)[%d %d]\n"
			,ctot,n,no,ii+1,jj);
		fflush(stderr);
		return;
	}
/*  Spatially resample. The sample length variable is cc which is  */ 
/*  continually incremented by Va[i] until it exceeds dC */
 
	cco = 0.;
	cc = va[ii];
	tc = 0.;
	to = 0.;

	xo[0] = x[ii];
	yo[0] = y[ii];
	zo[0] = z[ii];
	if(zo[0] > prsmin) {
		zo[0] = FEATCHAR_PENDOWN;
	} else {
		zo[0] = FEATCHAR_PENUP;
	}

	j=1;
	i=ii;
	while(i < (jj+1)) {
		if (cc >= dc) {
 
			/*  Calculate the actual time of reaching dC. */ 
			/*  Er is een lineaire functie cc(t) = a t + b die voorspelt */
			/*  wat de waarde van cc is als functie van de tijd. De inverse: */
			/*   t = (cc' - b) / a zegt mij op welk tijdstip cc(t) de waarde cc' */
			/*  bereikt */
 
			/*  a = cc(t) - cc(t-1) / (1) */ 
			/*  b = cc(t-1) - a * (1) */
 
			dc_rest = cc - dc;
			a = (cc - cco) / ((double) i - to) ;
			b = cco - a * to;
			tc = (dc - b) / a ;
 
			/*  and interpolate */ 
 
 			if (tc>=(double)n)
				tc = (double)n-1;
			xo[j] = recog_allo_interpolate(x,j,n,tc,(xo[j-1]+x_in[(int)tc])/2.);
			yo[j] = recog_allo_interpolate(y,j,n,tc,(yo[j-1]+y_in[(int)tc])/2.);
			zo[j] = recog_allo_interpolate(z,j,n,tc,(zo[j-1]+z[(int)tc])/2.);
			if(zo[j] > prsmin) {
				zo[j] = FEATCHAR_PENDOWN;
			} else {
				zo[j] = FEATCHAR_PENUP;
			}
 
			/*  The reservoir CC is set to the remainder and the output index */ 
			/*  is incremented */
 
			cc = dc_rest;
			cco = 0.;
			to = tc;
			++j;
			if( j >= no) break; /* LAMBERT */
		} else {
			cco = cc;
			to = (double) i;
			++i;
			cc = cc + va[i];
		}
	}
 
	if(j < no) { 
		xo[no-1] = x[jj];
		yo[no-1] = y[jj];
		zo[no-1] = z[jj];
		if(zo[no-1] > prsmin) {
			zo[no-1] = FEATCHAR_PENDOWN;
		} else {
			zo[no-1] = FEATCHAR_PENUP;
		}
	}
} /* End recog_spatial_z_sampler */

void recog_normalize_rms (
/*
	Normalize x and y to unit rms distance.
*/
int    nnorm,         /* Number of samples in xnorm and ynorm */
float  xnorm[NSMAX],  /* (input/output) */
float  ynorm[NSMAX])  /* (input/output) */
{
   double rmx,rmy,dd,d;
   int i;

	if(nnorm > 0) { 
		rmx = 0.0;
		rmy = 0.0;
		for(i=0; i < nnorm; ++i) {
			rmx=rmx+xnorm[i];
			rmy=rmy+ynorm[i];
		}
		rmx=rmx/nnorm;
		rmy=rmy/nnorm;
 
		dd=0.;
		for(i=0; i < nnorm; ++i) {
			dd=dd+(rmx-xnorm[i])*(rmx-xnorm[i])+(rmy-ynorm[i])*(rmy-ynorm[i]);
		}
 
		/*  norm factor d */ 
		 
		d=sqrt(dd/nnorm);
		if(d <= 0.) d=1.;
	 
		for(i=0; i < nnorm; ++i) {
			xnorm[i]=(xnorm[i]-rmx)/d;
			ynorm[i]=(ynorm[i]-rmy)/d;
		}

	} else { 
		fprintf(stderr,"%%recog_normalize_rms, no samples\n"); 
		fflush(stderr);
	}
} /* End recog_normalize_rms */




void allo_sampler_interface (int nsamples, int *xi, int *yi, int *zi
	, float *xo, float *yo, float *zo, int m, double prsmin)
{

float  x_in[NSMAX];  /* (input) movement x coordinate */
float  y_in[NSMAX];  /* (input) movement y coordinate */
float  z[NSMAX];     /* (input) movement z coordinate */
int    n;            /* Number of samples */
int    no;           /* Requested number of output samples */
float  va[NSMAX];    /* (output) absolute velocity (should be flat) */
int    ii,jj;        /* Beginning and end of strokes */

	int i;

	ii = 0;
	jj = n = nsamples;
	no = m;

	for (i=0;i<n;i++) {
		x_in[i] = (float) xi[i];
		y_in[i] = (float) yi[i];
		z[i]    = (float) zi[i];
	}
	recog_spatial_z_sampler(x_in,y_in,z,n,xo,yo,zo,no,va,ii,jj-1,prsmin);
	recog_normalize_rms(no,xo,yo);
}


void allo_double_sampler_interface (int nsamples, double *xi, double *yi, double *zi
	, double *xo, double *yo, double *zo, int m, double prsmin)
{

float  x_in[NSMAX];  /* (input) movement x coordinate */
float  y_in[NSMAX];  /* (input) movement y coordinate */
float  z_in[NSMAX];  /* (input) movement z coordinate */
float  x_out[NSMAX]; /* (input) movement x coordinate */
float  y_out[NSMAX]; /* (input) movement y coordinate */
float  z_out[NSMAX]; /* (input) movement z coordinate */
int    n;            /* Number of samples */
int    no;           /* Requested number of output samples */
float  va[NSMAX];    /* (output) absolute velocity (should be flat) */
int    ii,jj;        /* Beginning and end of strokes */

	int i;

	ii = 0;
	jj = n = nsamples;
	no = m;

	for (i=0;i<n;i++) {
		x_in[i] = (float) xi[i];
		y_in[i] = (float) yi[i];
		z_in[i]    = (float) zi[i];
	}
	recog_spatial_z_sampler(x_in,y_in,z_in,n,x_out,y_out,z_out,no,va,ii,jj-1,prsmin);
/*
	recog_normalize_rms(x_out,y_out,no);
*/
	for (i=0;i<no;i++) {
		xo[i] = (double) x_out[i];
		yo[i] = (double) y_out[i];
		zo[i] = (double) z_out[i];
	}
}


/* HOW TO NORMALIZE AN ALLOGRAPH:
 *
 * 1) normalize x,y 
 * 2) normalize z
*/

#define KWADRAAT(a) ((a)*(a))

void normalize_allo (double *v, int m)
/* copied from our recognizer */
{
	double rmx,rmy,dd,d;
	double z,zmin,zmax;
	int i;

fprintf (stderr,"normalizing allo,m=%d\n",m);
	rmx = rmy = 0.0;
	zmin = zmax = v[2];
	for (i=0;i<m;i++) {
		rmx += v[3*i];
		rmy += v[3*i+1];
		if ((z=v[3*i+2])<zmin) zmin = z;
		if (z>zmax) zmax = z;
	}
	rmx  /= m;
	rmy  /= m;
	
	dd = 0.0;

	if (zmax-zmin<.0001) {
		if (zmin>=1.0) {
			for (i=0;i<m;i++)
				v[3*i+2]=1.0;
		}
		else {
			for (i=0;i<m;i++)
				v[3*i+2] = 0.0;
		}
	}
	else {
		zmax -= zmin;
		for (i=0;i<m;i++) {
			z = v[3*i+2];
			if (zmax>.00001)
				v[3*i+2] = (z - zmin)/zmax;
		}
	}
	
	for (i=0;i<m;i++) {
		dd += KWADRAAT(rmx-v[3*i]) + KWADRAAT(rmy-v[3*i+1]);
	}
	d = sqrt(dd/m);
	if (d<=0.000001) d = 1.;

	for (i=0;i<m;i++) {
		v[3*i] = (v[3*i]-rmx)/d;
		v[3*i+1] = (v[3*i+1]-rmy)/d;
	}
}

void normalize_left (double *v, int m)
/* first coordinate is reference, no scale */
{
	double xleft,yleft;
	double z,zmin,zmax;
	int i;

fprintf (stderr,"normalize_left allo,m=%d\n",m);
	xleft = v[0];
	yleft = v[1];
	zmin = zmax = v[2];
	for (i=0;i<m;i++) {
		if ((z=v[3*i+2])<zmin) zmin = z;
		if (z>zmax) zmax = z;
	}

	if (zmax-zmin<.0001) {
		if (zmin>=1.0) {
			for (i=0;i<m;i++)
				v[3*i+2]=1.0;
		}
		else {
			for (i=0;i<m;i++)
				v[3*i+2] = 0.0;
		}
	} else {
		zmax -= zmin;
		for (i=0;i<m;i++) {
			z = v[3*i+2];
			if (zmax>.00001)
				v[3*i+2] = (z - zmin)/zmax;
		}
	}
	
	for (i=0;i<m;i++) {
		v[3*i] = (v[3*i]-xleft);
		v[3*i+1] = (v[3*i+1]-yleft);
	}
}

void normalize_middle (double *v, int m)
/* copied from our recognizer */
{
	double rmx,rmy;
	double z,zmin,zmax;
	int i;

fprintf (stderr,"normalizing middle,m=%d\n",m);
	rmx = rmy = 0.0;
	zmin = zmax = v[2];
	for (i=0;i<m;i++) {
		rmx += v[3*i];
		rmy += v[3*i+1];
		if ((z=v[3*i+2])<zmin) zmin = z;
		if (z>zmax) zmax = z;
	}
	rmx  /= m;
	rmy  /= m;
	
	if (zmax-zmin<.0001) {
		if (zmin>=1.0) {
			for (i=0;i<m;i++)
				v[3*i+2]=1.0;
		}
		else {
			for (i=0;i<m;i++)
				v[3*i+2] = 0.0;
		}
	}
	else {
		zmax -= zmin;
		for (i=0;i<m;i++) {
			z = v[3*i+2];
			if (zmax>.00001)
				v[3*i+2] = (z - zmin)/zmax;
		}
	}
	
	for (i=0;i<m;i++) {
		v[3*i] = (v[3*i]-rmx);
		v[3*i+1] = (v[3*i+1]-rmy);
	}
}

#define INDIVIDUAL_YLINE
#ifdef INDIVIDUAL_YLINE
double best_ybase(Yfont *yfont, double rmy, int npeaks_max) 
{ /* individual peaks, assume they are sorted  */
   int i, ibest, n;
   double d,dbest,ybest;
   
   ibest = -1;
   dbest = 1.e60;
   ybest = yfont->base;
   
   n = npeaks_max;
   if(n > yfont->npeaks) n = yfont->npeaks;
   for(i = 0; i < n; ++i) {
      d = yfont->ypeaklist[i] - rmy;
      if(d < 0.0) d = -d;

#ifdef YFONT_VERBOSE
      fprintf(stderr,"Peak %d is: y=%f ~= %d, rmy=%f d=%f\n"
                 , i, yfont->ypeaklist[i]
                 , yfont->ipeakpos[i]
                 , rmy, d);
#endif
      if(d < dbest) {
         dbest = d;
         ybest = yfont->ypeaklist[i];
         ibest = i;
      }
   }
   fprintf(stderr,"Peak %d is closest: y=%f rmy=%f d=%f\n"
                 , ibest, ybest, rmy, dbest);

   return(ybest);
}
#endif

int within_ybase_zone(Yfont *yfont, double rmy) 
{
   int within;
   double yup,ylo,z = 1.96;
   
   yup = yfont->base + z * yfont->sdb;
   ylo = yfont->base - z * yfont->sdb;
   
   if(rmy > ylo && rmy < yup) {
      within = 1;
   } else {
      within = 0;
   }
   fprintf(stderr,"ylo=%f ybase=%f yup=%f:  ym=%f --> within=%d\n"
                 , ylo,yfont->base,yup,rmy,within);
   return(within);
}

void normalize_midx_basy (double *v, int m, Yfont *yfont, int *within)
{
	double rmx,rmy,ybase;
	double z,zmin,zmax;
	int i;

fprintf (stderr,"normalizing midx, baseliney,m=%d\n",m);
	rmx = rmy = 0.0;
	zmin = zmax = v[2];
	for (i=0;i<m;i++) {
		rmx += v[3*i];
		rmy += v[3*i+1];
		if ((z=v[3*i+2])<zmin) zmin = z;
		if (z>zmax) zmax = z;
	}
	rmx  /= m;
	rmy  /= m;
	
	if (zmax-zmin<.0001) {
		if (zmin>=1.0) {
			for (i=0;i<m;i++)
				v[3*i+2]=1.0;
		}
		else {
			for (i=0;i<m;i++)
				v[3*i+2] = 0.0;
		}
	}
	else {
		zmax -= zmin;
		for (i=0;i<m;i++) {
			z = v[3*i+2];
			if (zmax>.00001)
				v[3*i+2] = (z - zmin)/zmax;
		}
	}
	
#ifdef INDIVIDUAL_YLINE
        ybase = best_ybase(yfont,rmy,NPEAKS_MAX);
#else
        ybase = yfont->base;
#endif
        
	for (i=0;i<m;i++) {
		v[3*i] = (v[3*i]-rmx);
		v[3*i+1] = (v[3*i+1] - ybase);
	}
	
        *within = within_ybase_zone(yfont,rmy);
}

/* August '97 (time flies)
 * input:  the original  (xi,yi,zi) from UNIPEN (n samples)
 *         the resampled (xo,yo,zo) (m samples)
 * output: the adjusted  (xo,yo,zo) (m samples)
 *
 * For each point in the original signal, the closest point in the
 * resampled signal is found. If the distance to this point divided
 * by the wanted distance between two resampled points is larger than
 * a factor, the point is substituted by the input point.
 */

int find_closest (double *dist, float x, float y, float *xo, float *yo, int m)
{
	double d,dmin,closest;
	int i;

	closest = 0;
	dmin = (x-xo[0])*(x-xo[0]) + (y-yo[0])*(y-yo[0]);
	for (i=1;i<m;i++) {
		if ((d=(x-xo[i])*(x-xo[i]) + (y-yo[i])*(y-yo[i]))<dmin) {
			closest = i;
			dmin = d;
		}
	}
	*dist = dmin;
	return closest;
}

void adjust_resampling (int ni, float *xi, float *yi, float *zi
   , int m, float *xo, float *yo, float *zo, double dfactor)
{
	int c,i,closest[NSMAX];
	double dist,dmax[NSMAX],dcrit,dx,dy,rtotal;
	int nmoved;

	rtotal = 0.0;
   for (i=0;i<ni;i++) {
		dx      = xi[i]-xi[i-1];
		dy      = yi[i]-yi[i-1];
		rtotal += sqrt(dx*dx+dy*dy);
	}

        dcrit = rtotal/(m-1); /* the wanted distance */
	if(dcrit<1.0E-10) {
		fprintf (stderr,"adjust_resampling: zero trajectory length!\n");
		return;
	}
	/* mark all output points as OK */
	for (i=0;i<m;i++) {
		closest[i] = -1;
		dmax[i] = 0.0;
	}
	/* For each point in the input, find the closest point in the output
	 * if the ratio dist/dcrit is above dfactor, mark that this point must
	 * be replaced by the input point, if it has not to be replaced already
	 * by an input-point with a LARGER distance
	 */

   for (i=0;i<ni;i++) {
		if (zi[i]<15.)
			continue;
		c = find_closest(&dist,(float)xi[i],(float)yi[i],xo,yo,m);
		if (dist/dcrit>=dfactor) {
			if (dist>dmax[c]) {
				dmax[c] = dist;
				closest[c] = i;
			}
			fprintf (stderr,"%f/%f = %f\n",dist,dcrit,dist/dcrit);
		}
	}

	/* now move all output points to input points, if it has to be done */
	nmoved = 0;
	for (i=0;i<m;i++) {
		if ((c=closest[i])!=-1) {
			nmoved += 1;
			fprintf (stderr,"moving %d to %d\n",i,c);
			xo[i]   = (float)xi[c];
			yo[i]   = (float)yi[c];
			zo[i]   = (float)zi[c];
		}
	}
	fprintf (stderr,"moved %d out of %d points\n",nmoved,m);
}

