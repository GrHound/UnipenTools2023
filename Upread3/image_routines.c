#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image_routines.h"
#include "bitmap_routines.h"

/* flood seed value heuristics */
#define sFirst  0
#define sTop    1
#define sBot    2
#define sRight  3
#define sLast   4
#define sN      5
#define sERR    5

#define NSAMPLE_TAPERED_TOWARDS_PENUP 5
#define NSAMPLE_FATTENED_AT_PENDOWN   2

#define PI ((double) 3.141592654)

IMG create_image(int width, int height) 
{
   IMG image;
   int i;

   image = (IMG) malloc (height*sizeof(int *));
   if(image == NULL) {
      fprintf(stderr,"Error malloc create_image()\n");
      exit(1);
   }
   for (i=0;i<height;i++) {
      image[i] = (int *) calloc (width,sizeof(int));
      if(image[i] == NULL) {
         fprintf(stderr,"Error malloc create_image(), row %d\n", i);
         exit(1);
      }
   }
   return(image);
}

void read_data_p3 (FILE *fp, IMG *image, int *w, int *h, double *Gmax)
{ /* ppm ascii */
   int gmax, ir, ig, ib;
   char buf[1024];
   int i,j;
 
   fscanf(fp,"%[^\n]\n", buf);
   fprintf(stderr,"Creator=[%s]\n", buf);

   fscanf(fp,"%d%d\n%d", w, h, &gmax);
   fprintf(stderr,"Gmax=%d w=%d, h=%d\n", (int) gmax, *w, *h); 
  
   *Gmax = gmax;
   
   *image = create_image(*w,*h);
  
   for (i = 0; i < *h; i++) {
      for (j = 0; j < *w; j++) {
      	 fscanf(fp,"%d%d%d", &ir, &ig, &ib);
         (*image)[i][j] = (ir + ig + ib)/3;      	 
      }
   }
   fprintf(stderr,"[Rdy P3]\n");
}



void read_data_p6 (FILE *fp, IMG *image, int *w, int *h, double *Gmax)
{ /* ppm raw */
   int gmax;
   Pix rgb[3];
   char dump[10];
   int i,j;
 
   fscanf(fp,"%d%d%d", w, h, &gmax);
   fprintf(stderr,"Gmax=%d w=%d, h=%d\n", (int) gmax, *w, *h); 
   fread(dump, 1, 1, fp);
  
   *Gmax = gmax;
   
   *image = create_image(*w,*h);
  
   for (i = 0; i < *h; i++) {
      for (j = 0; j < *w; j++) {
          fread(rgb, 3, 1, fp);
             (*image)[i][j] = ((int) rgb[0] + (int) rgb[1] + (int) rgb[2])/3;
      }
   }
   fprintf(stderr,"[Rdy P6]\n");
}


void read_data_p5 (FILE *fp, IMG *image, int *w, int *h, double *Gmax)
{ /* pgm raw */
   int gmax;
   char dump[100];
   int i,j;
   unsigned char *greyline;
 
   fscanf(fp,"%[^\n]\n", dump);
   while(dump[0] =='#') {
      if(fscanf(fp,"%[^\n]\n", dump) == EOF) {
         fprintf(stderr,"Premature EOF image\n");
         exit(1);
      }
   }
   fprintf(stderr,"[%s]\n", dump); 
   strcat(dump," -9999");
   sscanf(dump,"%d%d%d", w, h, &gmax);
   if(gmax <= 0) {
      fscanf(fp,"%d", &gmax);
   } 
   fread(dump, 1, 1, fp);
   
   fprintf(stderr,"Gmax=%d w=%d, h=%d\n", (int) gmax, *w, *h); 
  
   *Gmax = gmax;
   
   *image = create_image(*w,*h);
   
   greyline = (unsigned char *) malloc(*w * sizeof(unsigned char));
   if(greyline == NULL) {
      fprintf(stderr,"Error mallocing for PGM image w=%d h=%d\n",*w,*h);
      exit(1);
   }
  
   for (i = 0; i < *h; i++) {
      fread(greyline, *w, 1, fp);
      for (j = 0; j < *w; j++) {
               (*image)[i][j] = greyline[j];
      }
   }
   free(greyline);
   fprintf(stderr,"[Rdy P5]\n");
}

void read_data (char *fname, IMG *image, int *w, int *h, double *Gmax)
{
   FILE *fp;
   char type[20];
 
   if ((fp=fopen(fname,"r"))==NULL) {
      fprintf (stderr,"unable to open %s for input!!\n",fname);
      exit(1);
   }
   fscanf(fp,"%[^\n]\n", type);
   fprintf(stderr,"Type=[%s]\n", type);
   
   if(strcmp(type,"P3") == 0) {
          read_data_p3 (fp, image, w, h, Gmax);
      
   } else if(strcmp(type,"P5") == 0) { /* pgm raw ==> yields only V in HS[V] */
        read_data_p5 (fp, image, w, h, Gmax);

   } else if(strcmp(type,"P6") == 0) {
        read_data_p6 (fp, image, w, h, Gmax);
        
   } else {
        fprintf(stderr,"Unknown ppm type [%s]\n", type);
        exit(1);
   }
   
   fclose(fp);
   fprintf(stderr,"[Rdy Image read]\n");

}

void write_image(char *fname, IMG image, int w, int h, int frmt)
{
	switch (frmt) {
		case O_XBM:
			write_bitmap_data (fname,image,w,h);
			break;
		case O_PPM:
			write_data_ppm (fname,image,w,h);
			break;
		case O_PGM:
			write_data_pgm (fname,image,w,h);
			break;
		default:
			fprintf (stderr,"unknown format [%d] while trying to write image to '%s'\n"
				,frmt,fname);
			break;
	}
}


void write_merged_image (IMG image, int w, int h, char *fname)
{
	FILE *fp;
	unsigned char *pgm_row,*ptr;
	int gray_max,i,j;

	if ((fp=fopen(fname,"wb"))==NULL) {
		fprintf (stderr,"unable to open %s for output!!\n",fname);
		exit(1);
	}
	gray_max = 0;
	for (i=0;i<h;i++) {
		for (j=0;j<w;j++) {
			if (image[i][j]>gray_max) {
			   gray_max = image[i][j];
			}
		}
	}
	pgm_row = (unsigned char *) malloc (3*w);
	fprintf (fp,"P6\n# CREATOR: OME LAMPIE\n%d %d\n255\n",w,h);
	for (i=0;i<h;i++) {
		ptr = pgm_row;
		for (j=0;j<w;j++) {
			ptr[0] = ptr[1] = ptr[2] = (unsigned char) (256 - ( (int) (256.0/gray_max*image[i][j]) ));
			ptr += 3;
		}
		fwrite(pgm_row,sizeof(unsigned char),3*w,fp);
	}
	fclose(fp);
}

void write_bitmap_data (char *fname, IMG image, int w, int h)
{
	char **data;
	int i,j;

	data = (char **) malloc (h*sizeof(char *));
	for (i=0;i<h;i++) {
		data[i] = (char *) calloc (w,sizeof(char));
		for (j=0;j<w;j++)
			if (image[i][j])
				data[i][j] = 1;
	}
	write_bitmap (w,h,data,fname);
	for (i=0;i<h;i++)
		free(data[i]);
	free(data);
}

int get_random_grey(void) 
{
   int i;
   i = (int) (256. * drand48());
   if(i < 0) i = 0;
   if(i > 255) i = 255;
   return(i);
}

int get_mindif_col(ColPix *col)
{
   int mindif, d[3];
   
   d[0] = col->r - col->g;
   if(d[0] < 0) d[0] = -d[0];
   d[1] = col->g - col->b;
   if(d[1] < 0) d[1] = -d[1];
   d[2] = col->r - col->b;
   if(d[2] < 0) d[2] = -d[2];
   
   mindif = d[0];
   if(mindif > d[1]) mindif = d[1];
   if(mindif > d[2]) mindif = d[2];
   return(mindif);
}

void throw_random_color(ColPix *col)
{
   int mindif;
   
   mindif = -1;
   while(mindif < 30) {
         col->r = get_random_grey();
         col->g = get_random_grey();
         col->b = get_random_grey();
         mindif = get_mindif_col(col);
   }
}

void get_random_color(int iran, int *r, int *g, int *b)
{
   static ColPix coltable[256];
   static int init = 0;
   int i;
   
   if(init == 0) {
      init = 1;
      fprintf(stderr,"(generating random color table)\n");
      
      for(i = 0; i < 256; ++i) {
         throw_random_color(&coltable[i]);
      }
      
   }

   i = -iran;
   if(i < 0) i = 0;
   if(i > 255) i = 255;
   
   *r = coltable[i].r;   
   *g = coltable[i].g;   
   *b = coltable[i].b;   

#ifdef DEBUG   
   fprintf(stderr,"random %d ==> r=%d g=%d b=%d\n", i,*r,*g,*b);
#endif
}

void write_data_ppm (char *fname, IMG image, int w, int h)
{
	FILE *fp;
	unsigned char *pgm_row,*ptr;
	int i,j,r,g,b;

	if ((fp=fopen(fname,"wb"))==NULL) {
		fprintf (stderr,"unable to open %s for output!!\n",fname);
		exit(1);
	}
	pgm_row = (unsigned char *) malloc (3*w);
	fprintf (fp,"P6\n# CREATOR: OME LAMPIE\n%d %d\n255\n",w,h);
	for (i=0;i<h;i++) {
		ptr = pgm_row;
		for (j=0;j<w;j++) {
			if(image[i][j] == GREEN) {
			   *ptr++ = 91;
			   *ptr++ = 100;
			   *ptr++ = 100;
		        } else if(image[i][j] == RED) {
			   *ptr++ = 245;
			   *ptr++ = 119;
			   *ptr++ = 91;
		        } else if(image[i][j] == WHITE) {
			   *ptr++ = 255;
			   *ptr++ = 255;
			   *ptr++ = 255;
                        } else if(image[i][j] < 0) {
                            get_random_color(image[i][j],&r,&g,&b);
			    *ptr++ = r;
			    *ptr++ = g;
			    *ptr++ = b;
                            
                        } else {
			   ptr[0] = ptr[1] = ptr[2] = 
				   (unsigned char) ((int) (255 - image[i][j]));
			   ptr += 3;
			}
		}
		fwrite(pgm_row,sizeof(unsigned char),3*w,fp);
	}
	fclose(fp);
}

void write_data_pgm (char *fname, IMG image, int w, int h)
{
	FILE *fp;
	unsigned char *pgm_row,*ptr;
	int i,j;

	if ((fp=fopen(fname,"wb"))==NULL) {
		fprintf (stderr,"unable to open %s for output!!\n",fname);
		exit(1);
	}
	pgm_row = (unsigned char *) malloc (w * sizeof(unsigned char));
	fprintf (fp,"P5\n# CREATOR: OME LAMPIE\n%d %d\n255\n",w,h);
	for (i=0;i<h;i++) {
		ptr = pgm_row;
		for (j=0;j<w;j++) {
		   *ptr++ = 255 - image[i][j];
		}
		fwrite(pgm_row,sizeof(unsigned char),w,fp);
	}
	fclose(fp);
}

int find_non_col_right(IMG img, int w, int h, int x, int y, int col, int maxright, int *xx, int *yy)
{
   int i, j;
   
   for(i = y; i <= y; ++i) {
      for(j = x; j <= x+maxright; ++j) {
         if(i > 1 && i < h-1) {
             if(j > 1 && j < w-1) {
                if(img[i][j] != col) {
                   *xx = j;
                   *yy = i;
                   return(1);
                }
             }
         }
      }
   }
   return(0);
}

int find_non_col_left(IMG img, int w, int h, int x, int y, int col, int maxleft, int *xx, int *yy)
{
   int i, j;
   
   for(i = y; i <= y; ++i) {
      for(j = x; j >= x-maxleft; --j) {
         if(i > 1 && i < h-1) {
             if(j > 1 && j < w-1) {
                if(img[i][j] != col) {
                   *xx = j;
                   *yy = i;
                   return(1);
                }
             }
         }
      }
   }
   return(0);
}

int find_non_col_down(IMG img, int w, int h, int x, int y, int col, int maxdown, int *xx, int *yy)
{ 
   int i, j;
   
   for(i = y; i >= y-maxdown; --i) {
      for(j = x; j <= x; ++j) {
         if(i > 1 && i < h-1) {
             if(j > 1 && j < w-1) {
                if(img[i][j] != col) {
                   *xx = j;
                   *yy = i;
                   return(1);
                }
             }
         }
      }
   }
   return(0);
}

int find_non_col_up(IMG img, int w, int h, int x, int y, int col, int maxdown, int *xx, int *yy)
{ 
   int i, j;
   
   for(i = y; i <= y+maxdown; ++i) {
      for(j = x; j <= x; ++j) {
         if(i > 1 && i < h-1) {
             if(j > 1 && j < w-1) {
                if(img[i][j] != col) {
                   *xx = j;
                   *yy = i;
                   return(1);
                }
             }
         }
      }
   }
   return(0);
}

int flood_find_seed(IMG image, int width,int height
                             , int xs[], int ys[], int nseeds,int BRUSH
                             , int col,int *xseed, int *yseed)
{
    char *nms[] = {"First","Top","Right","Last","??"};                          
    int ifnd = sERR;
    
#define MAXRIGHT 1
#define MAXDOWN 1

    if(find_non_col_up(image,width,height,xs[sBot],ys[sBot]+BRUSH/2
                            ,col,MAXRIGHT,xseed,yseed)) {
      ifnd = sBot;

    } else if(find_non_col_right(image,width,height,xs[sFirst]+BRUSH/2,ys[sFirst]
                            ,col,MAXRIGHT,xseed,yseed)) {
      ifnd = sFirst;
      
    } else if(find_non_col_right(image,width,height,xs[sLast]+BRUSH/2,ys[sLast]
                            ,col,MAXRIGHT,xseed,yseed)) {
      ifnd = sLast;

    } else if(find_non_col_down(image,width,height,xs[sTop],ys[sTop]-BRUSH/2
                            ,col,MAXRIGHT,xseed,yseed)) {
      ifnd = sTop;

    } else if(find_non_col_left(image,width,height,xs[sTop]-BRUSH/2,ys[sTop]
                            ,col,MAXRIGHT,xseed,yseed)) {
      ifnd = sRight;
    } else {
      ifnd = sERR;
    }

    if(ifnd != sERR) {
       fprintf(stderr,"flood_fill, Start was at %s: x,y %d,%d, fill seed at %d %d for col %d\n"
                     ,nms[ifnd],xs[ifnd],ys[ifnd],*xseed,*yseed,col);
       return(1);
    } else {
       fprintf(stderr,"flood_fill, no seed found for col %d\n", col);
       return(0);
    }
}

int flood_fill(IMG img, int w, int h, int x, int y, int col, int depth, int maxdepth)
{
#define D 2

   if(depth > maxdepth) return;

   if(y > D && y < h-D) {
      if(x > D && x < w-D) {
          if(img[y][x] != col) {
              if(img[y][x] >= 0) {
                 img[y][x] = col; 

                 flood_fill(img,w,h,x+1,y  ,col, depth+1,maxdepth);
                 flood_fill(img,w,h,x  ,y-1,col, depth+1,maxdepth);
                 flood_fill(img,w,h,x-1,y  ,col, depth+1,maxdepth);
                 flood_fill(img,w,h,x  ,y+1,col, depth+1,maxdepth);
              }  
          }
      }
   }
}

double raised_cosine(double r)
{
   double g;
   
   if(r > 1.0) {
	g = 0.0;
   } else {
	g = 0.5 * (1. + cos(r * PI)); 
   }
   return(g);
}

double y_circle(double x, double r)
{
   double rx,y;
   
   if(r <= 0.0) {
      y = 1.0;  /* never loose contact */
   } else {
      if(x > r) {
  	  y = 0.0;
      } else {
          rx = x / r; /* relative to radius */
          y = 1.0 - rx * rx;
          if(y < 0.0) y = 0.0;
	  y = sqrt(y);
      }
   }
   return(y);
}

double brush_intensity(int x, int y, int BRUSH)
{
	int half;
	double midx,midy,dx,dy,g,r,rhalf;
	
	half = BRUSH / 2;
	rhalf = half;
	
	midx = rhalf;
	midy = rhalf;
	
	dx = x - midx;
	dy = y - midy;
	
	r = sqrt(dx*dx + dy*dy) / rhalf;

        g = raised_cosine(r);	
		
	return(g);
}

void init_brush(double a_brush[MAXBRUSH][MAXBRUSH], int BRUSH)
{
   int x,y;
   
   if(BRUSH > 1) {
     if(BRUSH == 2) {
	for(y = 0; y < BRUSH; ++y) {
	   for(x = 0; x < BRUSH; ++x) {
	   	 a_brush[y][x] = 1.;
	   }
	}
	
     } else {
	
	if(BRUSH > MAXBRUSH) {
		fprintf(stderr,"Max. brush ( -b %d ) is %d\n", BRUSH, MAXBRUSH);
		exit(1);
	}
	
#ifdef ODD_BRUSHES_ONLY
	if(BRUSH % 2 != 1) {
		fprintf(stderr,"Brush ( -b %d ) must be 1,2 or odd\n", BRUSH);
		exit(1);
	}
#endif
		
	for(y = 0; y < BRUSH; ++y) {
	   for(x = 0; x < BRUSH; ++x) {
	   	 a_brush[y][x] = brush_intensity(x,y,BRUSH);
	   }
	}
     }
   } else {
   	a_brush[0][0] = 1.0;
   }
}

void brush_addpix(IMG im, int w, int h, int x, int y, int col
            , double a_brush[MAXBRUSH][MAXBRUSH], int BRUSH)
{
        int lx, ly;
        int hx, hy;
        int x1, y1, x2, y2;
        int curcol, newcol;
   
	hy = (int) BRUSH/2;
	hx = (int) BRUSH/2;
	
	
	x1 = x - hx;
	x2 = x1 + BRUSH;
	
	y1 = y - hy;
	y2 = y1 + BRUSH;
	
	for (ly = y1; (ly < y2); ly++) {
		for (lx = x1; (lx < x2); lx++) {
		   if(ly >= 0 && ly < h) {
		      if(lx >= 0 && lx < w) {
			im[ly][lx] += (int) (a_brush[ly-y1][lx-x1]);
		      }
		   }
		}
	}
}

/* The Ink-Deposition Models. by Lambert Schomaker august 2006 */

double fountain_pen_wet_paper(int dx, int dy, int BRUSH, double v)
{
   double rdx, rdy, r, g, b, rBrush;
   int mid;

   /* Determine contact surface as a function of velocity.
      If velocity increases, it will decrease.
      We will prevent lift off and set the minimum
      width to two pixels */

   mid = BRUSH / 2;
   
   rBrush = (double) BRUSH * 5./(5.+v);
   if(rBrush < 2.0) rBrush = 2.0;		        

   /* Where are we (x,y) in the brush wrt the center,
      relative to the current brush radius? */
   
   rdx = (double) dx - (double) mid;
   rdy = (double) dy - (double) mid;
   r = sqrt(rdx*rdx + rdy*rdy) / (rBrush/2.);
	
   g = raised_cosine(r);	

   /* Reduce the deposition depending on speed,
      v=0 implies maximum deposition */
   
   b = g * exp(-v * 0.1);

   return(b);
}

double ballpoint(int dx, int dy, int BRUSH, double v)
{
   double rdx, rdy, r, g, b, rBrush, mid, rr, gboundary;
   
   /* pen parameters */
#define Vc 0.01 /* offset for maximum blockedness of pen tip: was 0.1 */
   double speed_dependent_deposition_eff    = 1.5; /* if 0.0: max eff. */   
   double speed_dependent_deposition_max    = 0.1; 
   double speed_dependent_radius            = 0.65;  
   double minimum_brush_width               = 3.0;  

   /* Determine contact surface as a function of velocity.
      If velocity increases, it will decrease.
      We will prevent lift off and set the minimum
      width to two pixels */

   mid = ((double) BRUSH - 1.0) / 2;
   
   rBrush = (double) BRUSH * (10.- speed_dependent_radius * v)/10.;
   if(rBrush < minimum_brush_width) rBrush = minimum_brush_width; 	        

   /* Where are we (x,y) in the brush wrt the center,
      relative to the maximum brush radius? */
   
   rdx = (double) dx - (double) mid;
   rdy = (double) dy - (double) mid;
   r = sqrt(rdx*rdx + rdy*rdy);
   
   if(r < rBrush/2.) {
      /* the ball has contact at this point (x,y) */
      /* this is the height at the edge of contact: */
      
      /* the smaller the dynamic radius, the heigher y of circle */
      
      gboundary = y_circle(rBrush/2.,mid);
      
      /* the more close the edge is to the physical fixed radius,
         the more stochastic is the deposition */
         
      g = 1.-gboundary + drand48() * gboundary;
      if(g > 0.5) {
         g = 1.0;
      } else {
         g = 0.0;
      }

   } else {
      g = 0.0;
   }

   /* Determine the satiation of the deposition process
      as a function of speed. At v=0, the deposition
      will be block shaped. At v->inf it will be narrow */

     b = pow(g,1./(Vc + speed_dependent_deposition_eff * v));
     
   /* Reduce the maximal deposition depending on speed,
      v=0 implies maximum deposition 1.0 */
   
     b = b * exp(-v * speed_dependent_deposition_max);

   return(b);
}

double fountain(int dx, int dy, int BRUSH, double v)
{ /* this was the first successful ink-deposition model */

   double rdx, rdy, r, g, b, rBrush, mid;
   
   /* pen parameters */
#define Vc 0.01 /* offset for maximum blockedness of pen tip: was 0.1 */
   double speed_dependent_deposition_eff    = 1.5; /* if 0.0: max eff. */   
   double speed_dependent_deposition_max    = 0.1; 
   double speed_dependent_radius            = 0.65;  /* was 0.7 */
   double minimum_brush_width               = 4.0;   /* was 2.0 */

   /* Determine contact surface as a function of velocity.
      If velocity increases, it will decrease.
      We will prevent lift off and set the minimum
      width to two pixels */

   mid = ((double) BRUSH - 1.0) / 2;
   
   rBrush = (double) BRUSH * (10.- speed_dependent_radius * v)/10.;
   if(rBrush < minimum_brush_width) rBrush = minimum_brush_width; 	        

   /* Where are we (x,y) in the brush wrt the center,
      relative to the current brush radius? */
   
   rdx = (double) dx - (double) mid;
   rdy = (double) dy - (double) mid;
   r = sqrt(rdx*rdx + rdy*rdy) / (rBrush/2.);
	
   g = raised_cosine(r);

   /* Determine the satiation of the deposition process
      as a function of speed. At v=0, the deposition
      will be block shaped. At v->inf it will be narrow */

     b = pow(g,1./(Vc + speed_dependent_deposition_eff * v));
     
   /* Reduce the maximal deposition depending on speed,
      v=0 implies maximum deposition 1.0 */
   
     b = b * exp(-v * speed_dependent_deposition_max);

   return(b);
}

double solid_ink_disk_brush(int dx,int dy,int BRUSH)
{
   double rdx, rdy, rhalf, r, b;
   
   rhalf = (double) (BRUSH-1) / 2.;
   rdx = (double) dx - rhalf;
   rdy = (double) dy - rhalf;
   r = sqrt(rdx*rdx + rdy*rdy);
   if(r <= rhalf) {
      b = 1.0; /* within circular brush: black */
   } else {
      b = 0.0; /* outside circular brush: white */
   } 
   return(b);
}

double solid_ink_disk_spray(int dx,int dy,int BRUSH, double pthresh)
{
   double rdx, rdy, rhalf, r, b;
   
   rhalf = (double) (BRUSH-1) / 2.;
   rdx = (double) dx - rhalf;
   rdy = (double) dy - rhalf;
   r = sqrt(rdx*rdx + rdy*rdy);
   if(r <= rhalf) {
      if(drand48() < pthresh) {
         b = 1.0; /* within circular brush: black */
      } else {
         b = 0.0;
      } 
   } else {
      b = 0.0; /* outside circular brush: white */
   } 
   return(b);
}

double calligrapher(int dx,int dy,int BRUSH, double angle_deg_in)
{
   double rdx, rdy, rhalf, r, b, angle, da, da2, da_thresh = 1., angle_deg;
   int i, j;
   
   angle_deg = - angle_deg_in; /* images are -y */
   rhalf = (double) (BRUSH-1) / 2.;
   
   rdx = (double) dx - rhalf;
   rdy = (double) dy - rhalf;
   if((int) rdx == 0 && (int) rdy == 0) {
      b = 1.0;
      return(b);
   }
   
   for(i = -1; i <= 1; ++i) {
      for(j = -1; j <= 1; ++j) {
        rdx = (double) (dx+i) - rhalf;
        rdy = (double) (dy+j) - rhalf;
   
        angle = 180. * atan2(rdy,rdx) / PI;
   
        da = angle - angle_deg;
        if(da < 0.0) da = - da;
   
        da2 = (angle - 180.) - angle_deg;
        if(da2 < 0.0) da2 = - da2;
   
        if(da2 < da) da = da2;
   
        if(da <= da_thresh) {
           b = 1.0; /* within narrow butterfly at angle_deg */
           return(b);
        }
      }
   }
   b = 0.0;
   return(b);
}

double ink_deposition(int dx, int dy, int BRUSH, double v, int inkmodel)
{
   double droplet_ink;
   
   if(inkmodel == INK_SOLID) {
      droplet_ink = solid_ink_disk_brush(dx,dy,BRUSH);

   } else if(inkmodel == INK_FOUNTAIN) {
      droplet_ink = fountain(dx,dy,BRUSH,v);

   } else if(inkmodel == INK_BALLPOINT) {
      droplet_ink = ballpoint(dx,dy,BRUSH,v);

   } else if(inkmodel == INK_FOUNTAINWETPAPER) {
      droplet_ink = fountain_pen_wet_paper(dx,dy,BRUSH,v);
      
   } else if(inkmodel == INK_CALLIGRAPHIC_45) {
      droplet_ink = calligrapher(dx,dy,BRUSH,45.);
      
   } else if(inkmodel == INK_SPRAY_DISK_50) {
      droplet_ink = solid_ink_disk_spray(dx,dy,BRUSH,0.10);
      
   } else {      
      fprintf(stderr,"Unknown ink model %d\n", inkmodel);
      exit(1);
   }
   return(droplet_ink);
}

/*-Ink deposition models */

void brush_grey(IMG im, int w, int h, int x, int y, int col
            , int BRUSH, double v
            , int inkmodel)
{
        int lx, ly;
        int hx, hy;
        int x1, y1, x2, y2;
        int curcol, newcol;
        double g;
   
	hy = (int) BRUSH/2;
	hx = (int) BRUSH/2;
	
	
	x1 = x - hx;
	x2 = x1 + BRUSH;
	
	y1 = y - hy;
	y2 = y1 + BRUSH;
	
	for (ly = y1; (ly < y2); ly++) {
		for (lx = x1; (lx < x2); lx++) {
		   if(ly >= 0 && ly < h) {
		      if(lx >= 0 && lx < w) {
			curcol = im[ly][lx];

			newcol = (int) ((double) col * 
			          ink_deposition(lx-x1,ly-y1,BRUSH,v,inkmodel));
			                 
		        /* Actual ink deposition: dark overwrites
		           lighter */	                 

#define MAXBLACK
#ifdef MAXBLACK
			if(newcol > curcol) {
				im[ly][lx] = newcol;
			}
#else /* MIXEDBLACK */
                        g = (double) newcol + 0.2 * (double) curcol;
		        if(g > curcol) {
                           if(g < 0.0) g = 0.0;
                           if(g > 255.0) g = 255.;
			   im[ly][lx] = (int) g;
                        } 
#endif
			
		      }
		   }
		}
	}
}

void brush_solid(IMG im, int w, int h, int x, int y, int col
            , double a_brush[MAXBRUSH][MAXBRUSH], int BRUSH)
{
        int lx, ly;
        int hx, hy;
        int x1, y1, x2, y2;
        int curcol, newcol;
   
	hy = (int) BRUSH/2;
	hx = (int) BRUSH/2;
	
	
	x1 = x - hx;
	x2 = x1 + BRUSH;
	
	y1 = y - hy;
	y2 = y1 + BRUSH;
	
	for (ly = y1; (ly < y2); ly++) {
		for (lx = x1; (lx < x2); lx++) {
		   if(ly >= 0 && ly < h) {
		      if(lx >= 0 && lx < w) {
			im[ly][lx] = col;
		      }
		   }
		}
	}
}

void brush(IMG im, int w, int h, int x, int y, int col
            , double a_brush[MAXBRUSH][MAXBRUSH]
            , int BRUSH, double v, int inkmodel)
{ /* v=0: maximum deposition, v > 0: decreasing deposition */
        int lx, ly;
        int hx, hy;
        int x1, y1, x2, y2;
        int curcol, newcol;
   
	hy = (int) BRUSH/2;
	hx = (int) BRUSH/2;
	
	
	x1 = x - hx;
	x2 = x1 + BRUSH;
	
	y1 = y - hy;
	y2 = y1 + BRUSH;
	
	if (col==ADD_PIXEL) {
           brush_addpix(im,w,h,x,y,col,a_brush,BRUSH);
        } else if(col < 0) {
           brush_solid(im,w,h,x,y,col,a_brush,BRUSH);
        } else {
           brush_grey(im,w,h,x,y,col,BRUSH,v,inkmodel);
        }		
}

void draw_line_in_image (IMG image, int width, int height, int x1, int y1, int x2, int y2, int col, int BRUSH, double v, int inkmodel)

/* stolen from gd.c by LOE */
/* Bresenham as presented in Foley & Van Dam */

{
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
        double a_brush[MAXBRUSH][MAXBRUSH];
        
       	init_brush(a_brush, BRUSH);
       	
        printf("Drawto: x=%d y=%d v=%.2f\n", x2,y2,v);

	dx = abs(x2-x1);
	dy = abs(y2-y1);
	if (dy <= dx) {
		d = 2*dy - dx;
		incr1 = 2*dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}
                brush(image,width,height,x,y,col,a_brush,BRUSH,v,inkmodel);
		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y++;
					d+=incr2;
				}
                                brush(image,width,height,x,y,col,a_brush,BRUSH,v,inkmodel);
			}
		} else {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y--;
					d+=incr2;
				}
                                brush(image,width,height,x,y,col,a_brush,BRUSH,v,inkmodel);
			}
		}		
	} else {
		d = 2*dx - dy;
		incr1 = 2*dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}
                brush(image,width,height,x,y,col,a_brush,BRUSH,v,inkmodel);
		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x++;
					d+=incr2;
				}
                                brush(image,width,height,x,y,col,a_brush,BRUSH,v,inkmodel);
			}
		} else {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x--;
					d+=incr2;
				}
                                brush(image,width,height,x,y,col,a_brush,BRUSH,v,inkmodel);
			}
		}
	}
}

void make_rectangles (IMG image, int x0, int y0, int N, int width, int height)
{
	int dx = width/N;
	int xorg,yorg;
        double a_brush[MAXBRUSH][MAXBRUSH];
        
	xorg = x0*dx/2;
	yorg = y0*dx/2;
	draw_line_in_image(image,width,height,xorg,yorg,xorg,yorg+dx-1,WHITE,1,0.0,INK_SOLID);
	draw_line_in_image(image,width,height,xorg,yorg+dx-1,xorg+dx-1,yorg+dx-1,WHITE,1,0.0,INK_SOLID);
	draw_line_in_image(image,width,height,xorg+dx-1,yorg+dx-1,xorg+dx-1,yorg,WHITE,1,0.0,INK_SOLID);
	draw_line_in_image(image,width,height,xorg+dx-1,yorg,xorg,yorg,WHITE,1,0.0,INK_SOLID);

       	init_brush(a_brush, 1);

	brush(image,width,height,xorg,yorg,GREEN,a_brush,1,0.0,INK_SOLID);
}

#ifdef OLD_TAPERING
int tapered_brush_towards_penup(int i, float z[], int n, int inbrush, int minpres)
{
   int bru, j, nfrom_penup;
   
   if(i <= NSAMPLE_FATTENED_AT_PENDOWN) {
      bru = inbrush + 1; /* First samples a little bit fatter */
      fprintf(stderr,"First samples a little bit fatter\n");
      
   } else {
      j = i + 1;
      while(j < n && z[j] > minpres) {
         ++j;
      }
      nfrom_penup = j - i;
      
      if(nfrom_penup < NSAMPLE_TAPERED_TOWARDS_PENUP) {
         bru = inbrush - (NSAMPLE_TAPERED_TOWARDS_PENUP-nfrom_penup-1);
         fprintf(stderr,"Tapered brush towards penup\n");
      } else {
         bru = inbrush;
      }
   }
   return(bru);
}
#endif

void plot_allo_in_image (IMG image, int width, int height, int n, int *x, int *y, int *z, int col, int BRUSH, int inkmodel)
{
	int i;
	int x0,y0,z0,x1,y1,z1;
	double v;

	x0 =  x[0];
	y0 =  y[0];
	z0 =  z[0];
	for (i=1;i<n;i++) {
		x1 =  x[i];
		y1 =  y[i];
		z1 =  z[i];
		v = sqrt((double) (x[i]-x[i-1]) * (x[i]-x[i-1])
		       + (double) (y[i]-y[i-1]) * (y[i]-y[i-1]));
		        
		if (z1>MINPRES) {
		   draw_line_in_image (image,width,height,x0,y0,x1,y1
		                                         ,col,BRUSH,v,inkmodel);
		}
		x0 = x1;
		y0 = y1;
		z0 = z1;
	}
}

void setval(int iarr[], int n, int ival)
{
   int i;
   for(i = 0; i < n; ++i) {
      iarr[i] = ival;
   }
}

void smooth_arr(float arrin[], float arrout[], int n) 
{
   int i;
   
   for(i = 1; i < n-1; ++i) {
      arrout[i] = (arrin[i-1] + arrin[i] + arrin[i+1])/3.;
   }
   arrout[0] = (arrin[0] + arrin[1])/2.;
   arrout[n-1] = (arrin[n-2] + arrin[n-1])/2.;
}

void nsmooth_arr(float arrin[], int n, int nsmo) 
{
   float *arr;
   int ismo,i;
   
   arr = (float *) malloc(n * sizeof(float));
   if(arr == NULL) {
      fprintf(stderr,"nsmooth_arr() Error malloc arr[]\n");
      exit(1);
   }
   
   for(ismo = 0; ismo < nsmo; ++ismo) {
      smooth_arr(arrin,arr,n);
      for(i = 0; i < n; ++i) {
         arrin[i] = arr[i];
      }
   }

   free(arr);
}

void smooth_xy(float x[], float y[], int n, int nsmo) 
{
   if(nsmo <= 1 || n < 3) return;
   
   nsmooth_arr(x,n,nsmo);
   nsmooth_arr(y,n,nsmo);
}

void float_plot_allo_in_image_actual(IMG image, int width, int height, int n
	, float x[], float y[], float z[]
	, int col, int BRUSH, int filled, int inkmodel)
{
	int i;
	int x0,y0,z0,x1,y1,z1,bru,xseed,yseed;
	int xs[sN], ys[sN], beg, ytop, ybot, xright;
	double v;
	
        fprintf(stderr,"float_plot_allo_in_image()\n");

	x0 =  (int) x[0];
	y0 =  (int) y[0];
	z0 =  (int) z[0];
	
        setval(xs,sN,x0);
        setval(ys,sN,y0);
        
        xright = -99999999;
        ytop = -99999999;
        ybot = 99999999;
	
        beg = 0;
	for (i=1;i<n;i++) {
	        if(beg == 0) {
	          if(z0 > 0.0) {
	             xs[sFirst] = x0;
	             ys[sFirst] = y0;
	             beg = 1;
	             
	             if(x0 > xright) {
	                xright = x0;
	                xs[sRight] = x0;
	                ys[sRight] = y0;
	             }
	             if(y0 > ytop) { 
	                ytop = y0;
	                xs[sTop] = x0;
	                ys[sTop] = y0;
	             }
	             if(y0 < ybot) { 
	                ytop = y0;
	                xs[sBot] = x0;
	                ys[sBot] = y0;
	             }
	          }
	        } 
	
		x1 =  (int) x[i];
		y1 =  (int) y[i];
		z1 =  (int) z[i];

#ifdef DEBUG
		fprintf(stderr,"%d %d %d\n", x1,y1,z1);
#endif

		if (z1>MINPRES) {
		        if(inkmodel != INK_SOLID) {
#ifdef OLD_TAPERING
/* OLD: primitive tapering: */
		           bru = tapered_brush_towards_penup(i,z,n,BRUSH,MINPRES);
#endif        
  		           v = sqrt((double) (x1-x0) * (x1-x0)
		                  + (double) (y1-y0) * (y1-y0));
		           bru = BRUSH;
		        } else {
		           v = 0.0;
		           bru = BRUSH;
		        }
		        
			draw_line_in_image (image,width,height,x0,y0,x1,y1,col,bru,v,inkmodel);
		}

	        if(z0 > 0.0) {	           
	           xs[sLast] = x0;
	           ys[sLast] = y0;
	        }

		x0 = x1;
		y0 = y1;
		z0 = z1;
	}

        if(filled) {
           /* close trace */
	   draw_line_in_image(image,width,height,xs[sLast],ys[sLast]
	                                 ,xs[sFirst],ys[sFirst],col,BRUSH,0.0,INK_SOLID); 

           fprintf(stderr,"flood_fill()\n");
           if(flood_find_seed(image,width,height,xs,ys,sN,BRUSH
                                                ,col,&xseed,&yseed)) {
#define MAXDEPTH 100000
                            
              flood_fill(image,width,height,xseed,yseed,col,0,MAXDEPTH);
           }
        }
}

void float_plot_allo_in_image (IMG image, int width, int height, int n
	, float *x, float *y, float *z
	, int col, int BRUSH, int filled, int nsmo, int inkmodel)
{
   if(nsmo > 1) {
      smooth_xy(x,y,n,nsmo);
   }
   float_plot_allo_in_image_actual(image,width,height,n,x,y,z,col,BRUSH,filled,inkmodel);
}

#define KWADRAAT(a) ((a)*(a))
void float_normalize_allo_xy (int n, float *x, float *y, float *xo, float *yo)
{
	double rmx,rmy,dd,d;
	int i;

	rmx = rmy = 0.0;
	for (i=0;i<n;i++) {
		rmx += x[i];
		rmy += y[i];
	}
	rmx  /= n;
	rmy  /= n;
	
	dd = 0.0;

	for (i=0;i<n;i++) {
		dd += KWADRAAT(rmx-x[i]) + KWADRAAT(rmy-y[i]);
	}
	d = sqrt(dd/n);
	if (d<=0.000001) d = 1.;

	for (i=0;i<n;i++) {
		xo[i] = (double) (x[i]-rmx) / d;
		yo[i] = (double) (y[i]-rmy) / d;
	}
}

void normalize_allo_xy (int n, int *x, int *y, double *xo, double *yo)
{
	double rmx,rmy,dd,d;
	int i;

	rmx = rmy = 0.0;
	for (i=0;i<n;i++) {
		rmx += x[i];
		rmy += y[i];
	}
	rmx  /= n;
	rmy  /= n;
	
	dd = 0.0;

	for (i=0;i<n;i++) {
		dd += KWADRAAT(rmx-x[i]) + KWADRAAT(rmy-y[i]);
	}
	d = sqrt(dd/n);
	if (d<=0.000001) d = 1.;

	for (i=0;i<n;i++) {
		xo[i] = (double) (x[i]-rmx) / d;
		yo[i] = (double) (y[i]-rmy) / d;
	}
}

#define XS(x) (xoffset+((x-xmin)/xrange-.5)*xfactor*w)
#define YS(y) (height-yoffset-((y-ymin)/yrange-.5)*yfactor*h)

void check_pix_fit(int x[],int y[],int n,int width,int height, int wtablet, int htablet) {
	int i;
	
	for(i = 0; i < n; ++i) {
		if(x[i] < 0 || x[i] > width - 1) {
			fprintf(stderr,"Pixel overflow on x=%d <0 or >width=%d, enlarge range (arg -r) Culprit: w=%d\n", x[i], width,wtablet);
			exit(1);
		}
		if(y[i] < 0 || y[i] > height - 1) {
			fprintf(stderr,"Pixel overflow on y=%d <0 or >height=%d, enlarge range (arg -r) Culprit: h=%d\n", y[i],height,htablet);
			exit(1);
		}
	}
}

void float_check_pix_fit(float x[],float y[],int n,int width,int height, int wtablet, int htablet) {
	int i;
	
	for(i = 0; i < n; ++i) {
		if(x[i] < 0 || x[i] > width - 1) {
			fprintf(stderr,"Pixel overflow on x=%f <0 or >width=%d, enlarge range (arg -r) Culprit: w=%d\n", x[i], width,wtablet);
			exit(1);
		}
		if(y[i] < 0 || y[i] > height - 1) {
			fprintf(stderr,"Pixel overflow on y=%f <0 or >height=%d, enlarge range (arg -r) Culprit: h=%d\n", y[i],height,htablet);
			exit(1);
		}
	}
}


void scale_allo (int n, double *xi, double *yi, int *x, int *y, int width, int height, int margin, double range)
{
	double xmin,xmax,ymin,ymax;
	double xrange,yrange,xfactor,yfactor,w,h;
	double xoffset, yoffset, xmid, ymid;
	int i;

	xmin = xmax = xi[0];
	ymin = ymax = yi[0];
	for (i=1;i<n;i++) {
		if (xmin>xi[i]) xmin = xi[i];
		if (xmax<xi[i]) xmax = xi[i];
		if (ymin>yi[i]) ymin = yi[i];
		if (ymax<yi[i]) ymax = yi[i];
	}
	w = width  - margin;
	h = height - margin;
	
	xoffset = width/2.0;
	yoffset = height/2.0;
	if(range <= 0.0) {
	   xrange = xmax-xmin;
	   yrange = ymax-ymin;

	   if (xrange/w > yrange/h) {
		xfactor = 1.;
		yfactor = yrange/xrange;
		if(yfactor * w > h) {
			w = h;
		} else {
			h = w;
		}
	   } else {
		xfactor = xrange/yrange;
		yfactor = 1.;
		if(xfactor * h > w) {
			h = w;
		} else {
			w = h;
		}
	   }
	   for (i=0;i<n;i++) {
		x[i] = (int)XS(xi[i]);
		y[i] = (int)YS(yi[i]);
	   }
	} else {
	   xrange = range;
	   yrange = range;
	   
	   xmid = (xmin + xmax)/2.;
	   ymid = (ymin + ymax)/2.;
	   
	   for (i=0;i<n;i++) {
		x[i] = xoffset + (double) width * (xi[i] - xmid) / range;
		y[i] = height - (yoffset + (double) width * (yi[i] - ymid) / range);
	   }
	}
	check_pix_fit(x,y,n,width,height,xmax-xmin, ymax-ymin);
}


void float_scale_allo (int n
        , float *xi, float *yi, float *zi
	, float *x, float *y, float *z
	, int width, int height, int margin
	, double range, int scalemode)
{
   double xmin,xmax,ymin,ymax;
   double xrange,yrange,xfactor,yfactor,w,h;
   double xoffset,yoffset,xmid,ymid;
   int i;
   
   if(scalemode == 1) {

	xmin = xmax = xi[0];
	ymin = ymax = yi[0];
	for (i=0;i<n;i++) {
#ifdef VERBOSE
	fprintf(stderr,"Tablet sample %d x=%f y=%f z=%f\n"
	              , i, xi[i],yi[i],zi[i]);
#endif
           if(zi[i] > 0.0) {
		if (xmin > xi[i]) xmin = xi[i];
		if (xmax < xi[i]) xmax = xi[i];
		if (ymin > yi[i]) ymin = yi[i];
		if (ymax < yi[i]) ymax = yi[i];
	   }
	}
	fprintf(stderr,"Tablet (UNIPEN) space w=%f h=%f\n", xmax-xmin, ymax-ymin);
	fprintf(stderr,"Tablet (UNIPEN) space xmin=%f xmax=%f\n", xmin,xmax);
	fprintf(stderr,"Tablet (UNIPEN) space ymin=%f ymax=%f\n", ymin, ymax);
	fflush(stderr);
	
	w = width  - margin;
	h = height - margin;
	
	xoffset = width/2.0;
	yoffset = height/2.0;
	
	if(range <= 0.0) {
	    xrange = xmax-xmin;
	    yrange = ymax-ymin;
		
	    if (xrange/w > yrange/h) {
		xfactor = 1.;
		yfactor = yrange/xrange;
		if(yfactor * w > h) {
			w = h;
		} else {
			h = w;
		}
	    } else {
		xfactor = xrange/yrange;
		yfactor = 1.;
		if(xfactor * h > w) {
			h = w;
		} else {
			w = h;
		}
	    }

	    for (i=0;i<n;i++) {
		x[i] = XS(xi[i]);
		y[i] = YS(yi[i]);
		z[i] = zi[i];
	    }
	} else {
	   xoffset = width/2.0;
	   yoffset = height/2.0;
	   xrange = range;
	   yrange = range;
	   
	   xmid = (xmin + xmax)/2.;
	   ymid = (ymin + ymax)/2.;
	   
	   for (i=0;i<n;i++) {
		x[i] = xoffset + (double) width * (xi[i] - xmid) / range;
		y[i] = height - (yoffset + (double) width * (yi[i] - ymid) / range);
		z[i] = zi[i];
	   }
	}
	float_check_pix_fit(x,y,n,width,height,xmax-xmin, ymax-ymin);
   } else {
        /* RAW copy of UNIPEN-scaled coordinates 
           (e.g. canvas mode with scanned text image
            as background) */ 
            
        yoffset = 10000; /* HACK */
     
	for (i=0;i<n;i++) {
	   x[i] = xi[i];
	   y[i] = yoffset - yi[i];
	   z[i] = zi[i];
	}
   }
}

int determine_brush (int xmin,int xmax,int ymin,int ymax,int width,int height)
{
	double r;
	int brush;

	if (xmax-xmin>ymax-ymin)
		r = xmax-xmin;
	else
		r = ymax-ymin;
	brush = (int) (.2*50*width/r);
	if (brush<MIN_BRUSH) {
		return MIN_BRUSH;
	} else
		return brush;
}

int count_pixels_in_sub_image (IMG image, double *X, double *Y, int x0, int y0, int N, int width)
{
	int dx = width/N;
	int i,j;
	double x,y;
	int xorg,yorg;
	int npixels = 0;

	xorg = x0*dx/2;
	yorg = y0*dx/2;
	for (i=xorg;i<xorg+dx;i++)
		for (j=yorg;j<yorg+dx;j++) {
			if (image[j][i]==RED) {
				x = (double) (i-xorg)/dx;
				y = (double) (j-yorg)/dx;
				X[npixels] = x;
				Y[npixels] = y;
				npixels++;
			}
	}
	return npixels;
}

void visualize_image (IMG image, int w, int h, int dowait)
{
	char cmd[256];

	if (dowait)
		sprintf (cmd,"xv /tmp/loetje.pgm");
	else
		sprintf (cmd,"xv /tmp/loetje.pgm &");
	write_data_pgm("/tmp/loetje.pgm",image,w,h);
	system (cmd);
}

void clear_image (IMG image, int w, int h)
{
	int i;

	for (i=0;i<h;i++) {
		memset((void *)image[i],0,w*sizeof(int));
	}
}
