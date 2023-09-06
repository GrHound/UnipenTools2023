#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#define BIGCHUNK  "PAGE"
#define COCOCHUNK "WORD"

#define PENUP   0
#define PENDOWN 1

#define SCALE_AS_IS          1
#define SCALE_LEFT           2
#define SCALE_RECO_SCALE     3
#define SCALE_RECO_POS_SCALE 4
#define SCALE_XT             5
#define SCALE_YT             6
#define SCALE_FT             7

#define NCHARBUF 10000

int get_property(char *instance, char *prop)
{
   char buf[NCHARBUF];
   char val[NCHARBUF];
   char *eop;
   int ival = 0;
   
   eop = strstr(instance, prop);
   
   if(eop == NULL) {
      fprintf(stderr,"Property [%s] not found in instance code [%s]\n"
                    , prop, instance);
      exit(1);
   }
   strcpy(buf, eop);
   eop = strchr(buf, '=');
   if(eop == NULL) {
      fprintf(stderr,"Property [%s] has no '='\n", prop);
      exit(1);
   }
   strcpy(val,eop+1);
   eop = strchr(val, '-');
   if(eop != NULL) {
      *eop = ' ';
   }
   eop = strchr(val, '/');
   if(eop != NULL) {
      *eop = ' ';
   }
   sscanf(val,"%d", &ival);
/*
   fprintf(stderr,"Prop [%s]=[%s]=%d\n", prop, val, ival);
 */  
   return(ival);
}

double halfroot(double ww, double hh)
{
   double w, h;
    
   w = ww/2.;
   h = hh/2.;
   return(sqrt(h*h+w*w)/sqrt(2));
}

void handle_line (char *line, int row, int col, int nfeat, int uselabel, double fact, int scalemode)
{
	char label[NCHARBUF];
	double x,y,xleft,yleft;
	double xoff = 0.0,yoff = 0.0,w = 1.0,h = 1.0,y1=0.0,y2=1000.;
	char *newptr,*ptr;
	int i;

	ptr = line;
	if(uselabel) {
	   if (sscanf(ptr,"%s",label)!=1) {
		fprintf (stderr,"error in line, unable to determine label!\n");
		fprintf (stderr,"line was:\n%s\n",line);
		exit(1);
	   }
	   ptr += strlen(label)+1;
	   if(scalemode == SCALE_RECO_SCALE
	   || scalemode == SCALE_RECO_POS_SCALE) {
	      y1 = get_property(label,"y1=");
	      y2 = get_property(label,"y2=");
	      xoff = get_property(label,"x=");
	      yoff = get_property(label,"y=");
	      w = get_property(label,"w=");
	      h = get_property(label,"h=");
	       
#ifdef DEBUG
	      fprintf(stderr,"xoff=%f yoff=%f w=%f h=%f\n"
	                    , xoff, yoff, w, h); 
#endif
	     
	   }
	} else {
	   if(scalemode == SCALE_RECO_SCALE 
	   || scalemode == SCALE_RECO_POS_SCALE) {
	      fprintf(stderr,"Cannot use scalemode reco... without COCO label with properties\n");
	      exit(1);
	   }
	   /* ptr = ok */
	   sprintf(label,"(%02d,%02d)", row, col);
	}
	printf (".SEGMENT %s ? ? \"%s\"\n",COCOCHUNK,label);
	printf (".PEN_DOWN\n");

	newptr = ptr;
	x = strtod(ptr,&newptr);
	xleft = x;
	yleft = 0.0;
	i = 1;
	while (ptr != newptr && i <= nfeat) {
		ptr = newptr;
		y = strtod(ptr,&newptr);
		if(i == 1) {
		  yleft = y;
	        }
		++i;
		if (ptr == newptr) {
			fprintf (stderr,"unable to determine Y coordinate at [%s]!!\n", ptr);
			if(scalemode != SCALE_FT) {
			   fprintf (stderr,"FATAL\n");
			   exit(1);
			} else {
			   fprintf (stderr,"... in FT: just double last feature value as pseudo-Y\n");
			   fprintf (stderr,"... (hey, the ft scale mode  is for inspection)\n");
			   y = x;
			}
		}
		ptr = newptr;
		if(scalemode == SCALE_AS_IS) {
		   printf("%.0f %.0f\n", fact * x, fact * y);

	        } else if(scalemode == SCALE_XT) {
		   printf("%d %.0f\n", i, fact * x);
		   
	        } else if(scalemode == SCALE_YT) {
		   printf("%d %.0f\n", i, fact * y);
		   
	        } else if(scalemode == SCALE_FT) {
		   printf("%d %.0f\n", 2*i, fact * x);
		   printf("%d %.0f\n", 2*i+1, fact * y);
		   
		} else if(scalemode == SCALE_LEFT) {
		   printf("%.0f %.0f\n", fact * (x-xleft), fact * y);
		   
	        } else if(scalemode == SCALE_RECO_SCALE) {
		   printf("%.0f %.0f\n",  (x-xleft)*h
		                       ,  (y-yleft)*h);
	        } else if(scalemode == SCALE_RECO_POS_SCALE) {
	           fact = halfroot(w,h);
		   printf("%.0f %.0f\n", xoff + x * fact + w/2.
		                       , (y2-y1) - yoff + y * fact - h/2. + 10000 - (y2-y1));
#ifdef WAS_OK
		                       , (y2-y1) - yoff + y * fact - h/2.);
#endif
	        }

		x = strtod(ptr,&newptr);
		++i;
	}
	printf (".PEN_UP\n");
}

int main(int argc, char *argv[])
{
	char line[100000];
	int nw, nh, nfeat, i, j, label, skipline, scalemode;
	double fact;
	FILE *fp;
	
	if(argc != 9) {
	   fprintf(stderr,"Usage: contoursom2unipen nw nh");
	   fprintf(stderr," nfeat fact somfile.feat");
	   fprintf(stderr," (no)label (no)skipline");
	   fprintf(stderr," scalemode(asis|left|reco_scale|reco_pos_scale|xt|yt)");
	   fprintf(stderr," > somfile.unp\n");
		exit(1);
	}
	nw = atoi(argv[1]);
	nh = atoi(argv[2]);
	nfeat = atoi(argv[3]);
	fact = atof(argv[4]);
	
	fp = fopen(argv[5],"r");
	if(fp == NULL) {
		fprintf(stderr,"Error opening [%s] for read\n", argv[5]);
		exit(1);
	}
	
	if(strcmp(argv[6],"label") == 0) {
		label = 1;
	} else {
		label = 0;
	}
	if(strcmp(argv[7],"skipline") == 0) {
		skipline = 1;
	} else {
		skipline = 0;
	}
	if(strcmp(argv[8],"asis") == 0) {
		scalemode = SCALE_AS_IS;
		fprintf(stderr,"(scalemode as is)\n");
	} else if(strcmp(argv[8],"xt") == 0) {
		scalemode = SCALE_XT;
		fprintf(stderr,"(scalemode X(t))\n");
	} else if(strcmp(argv[8],"yt") == 0) {
		scalemode = SCALE_YT;
		fprintf(stderr,"(scalemode Y(t))\n");
	} else if(strcmp(argv[8],"ft") == 0) {
		scalemode = SCALE_FT;
		fprintf(stderr,"(scalemode f(t))\n");
	} else if(strcmp(argv[8],"left") == 0) {
		scalemode = SCALE_LEFT;
		fprintf(stderr,"(scalemode left)\n");
	} else if(strcmp(argv[8],"reco_scale") == 0) {
	        scalemode = SCALE_RECO_SCALE;
		fprintf(stderr,"(scalemode reconstruct scale)\n");
	} else if(strcmp(argv[8],"reco_pos_scale") == 0) {
	        scalemode = SCALE_RECO_POS_SCALE;
		fprintf(stderr,"(scalemode reconstruct position and scale)\n");
	} else {
	        fprintf(stderr,"Unknown scale mode %s\n", argv[8]);
	        exit(1);
	}

	printf (".VERSION 1.0\n");
	printf (".HIERARCHY %s %s\n", BIGCHUNK, COCOCHUNK);
	printf (".COORD X Y \n");
	printf (".COMMENT This sample rate is fake for contours derived from the image:\n");
	printf (".POINTS_PER_SECOND 100 \n");
	printf (".SEGMENT %s ? ? \"%s\" \n", BIGCHUNK, argv[5]);
	
	i = 0;
	j = 0;
	while(fgets(line,16384,fp)!=NULL) {
		if(! skipline) {
		   handle_line(line,i,j,nfeat,label,fact,scalemode);
		   ++j;
		   if(j >= nw) {
			j = 0;
			++i;
		   }
		} else {
		   skipline = 0;
	        }
	}
	return 0;
}
