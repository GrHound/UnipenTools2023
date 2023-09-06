#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

#define MAX_SAMPLES 65536

static char indexfile[512];

void print_options (void)
{
	fprintf (stderr,"options are:\n");
	fprintf (stderr,"     -o  epsfile\n");
	fprintf (stderr,"     -P  patfile\n");
	fprintf (stderr,"     -b  boxsize\n");
	fprintf (stderr,"     -ys ystep\n");
	fprintf (stderr,"     -W  width\n");
	fprintf (stderr,"     -H  height\n");
	fprintf (stderr,"     -f  fontsize\n");
	fprintf (stderr,"     -c  ncolumns\n");
	fprintf (stderr,"     -s  string\n");
	fprintf (stderr,"     -i  index\n");
	fprintf (stderr,"     -I  index file\n");
	fprintf (stderr,"     -l  level\n");
	fprintf (stderr,"     -m  margin (fraction, say .05)\n");
	fprintf (stderr,"     -F  first\n");
	fprintf (stderr,"     -L  last\n");
	fprintf (stderr,"     -mp minpres\n");
	fprintf (stderr,"     -pd width (width of pendown strokes)\n");
	fprintf (stderr,"     -pu width (width of penup strokes)\n");
	fprintf (stderr,"     -p  point_size (radius of sample points)\n");
	fprintf (stderr,"     -S  (for same scale)\n");
	fprintf (stderr,"     -PX page-offset X-direction\n");
	fprintf (stderr,"     -PY page-offset Y-direction\n");
	fprintf (stderr,"     -LX label-offset X-direction\n");
	fprintf (stderr,"     -LY label-offset Y-direction\n");
	fprintf (stderr,"     -NL (for no label)\n");
}

static upsegQueryInfo *parse_args (int argc, char *argv[], char epsfname[])
{
	int i;
	static upsegQueryInfo info;

	if (argc<2) {
		fprintf (stderr,"use: %s unipen-file [unipen-files] [options]!\n",argv[0]);
		print_options();
		exit(1);
	}

	/* set default values */

	upsegInitializeQueryInfo(&info);
	strcpy(epsfname,"tmp.eps");

	info.ncolumns = -1;
	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-S")==0) {
			info.same_scale = 1;
			fprintf (stderr,"scaling set to same scale\n");
		}
		else if (strcmp(argv[i],"-P")==0) {
			i++;
			fprintf (stderr,"using '%s' for patfile\n",argv[i]);
			upsegAddQueriesFromPatfile(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-o")==0) {
			i++;
			strcpy(epsfname,argv[i]);
			fprintf (stderr,"using '%s' for output\n",epsfname);
		}
		else if (strcmp(argv[i],"-NL")==0) {
			info.nolabel = 1;
			fprintf (stderr,"no labels outputed\n");
		}
		else if (strcmp(argv[i],"-LX")==0) {
			i++;
			info.label_offsetx = atoi(argv[i]);
			fprintf (stderr,"label_offsetx set to %d\n",info.label_offsetx);
		}
		else if (strcmp(argv[i],"-LY")==0) {
			i++;
			info.label_offsety = atoi(argv[i]);
			fprintf (stderr,"label_offsety set to %d\n",info.label_offsety);
		}
		else if (strcmp(argv[i],"-PX")==0) {
			i++;
			info.page_offsetx = atoi(argv[i]);
			fprintf (stderr,"page_offsetx set to %d\n",info.page_offsetx);
		}
		else if (strcmp(argv[i],"-PY")==0) {
			i++;
			info.page_offsety = atoi(argv[i]);
			fprintf (stderr,"page_offsety set to %d\n",info.page_offsety);
		}
		else if (strcmp(argv[i],"-p")==0) {
			i++;
			info.pointsize = atoi(argv[i]);
			fprintf (stderr,"pointsize set to %d\n",info.pointsize);
		}
		else if (strcmp(argv[i],"-f")==0) {
			i++;
			info.fontsize = atoi(argv[i]);
			fprintf (stderr,"fontsize set to %d\n",info.fontsize);
		}
		else if (strcmp(argv[i],"-W")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.width);
			fprintf (stderr,"bounding box width set to %f\n",info.width);
		}
		else if (strcmp(argv[i],"-H")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.height);
			fprintf (stderr,"bounding box height set to %f\n",info.height);
		}
		else if (strcmp(argv[i],"-pu")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.pu);
			fprintf (stderr,"penup width set to %f\n",info.pu);
		}
		else if (strcmp(argv[i],"-pd")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.pd);
			fprintf (stderr,"pendown width set to %f\n",info.pd);
		}
		else if (strcmp(argv[i],"-mp")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.minpres);
			fprintf (stderr,"minpres set to %f\n",info.minpres);
		}
		else if (strcmp(argv[i],"-m")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.margin);
			fprintf (stderr,"margin set to %f\n",info.margin);
		}
		else if (strcmp(argv[i],"-x")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.xscale);
			fprintf (stderr,"xscale set to %f\n",info.xscale);
		}
		else if (strcmp(argv[i],"-ys")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.ystep);
			fprintf (stderr,"ystep set to %f\n",info.ystep);
		}
		else if (strcmp(argv[i],"-y")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.yscale);
			fprintf (stderr,"yscale set to %f\n",info.yscale);
		}
		else if (strcmp(argv[i],"-b")==0) {
			i++;
			sscanf(argv[i],"%lf",&info.boxsize);
			fprintf (stderr,"boxsize set to %f\n",info.boxsize);
		}
		else if (strcmp(argv[i],"-c")==0) {
			i++;
			info.ncolumns = atoi(argv[i]);
			fprintf (stderr,"ncolumns set to %d\n",info.ncolumns);
		}
		else if (strcmp(argv[i],"-s")==0) {
			i++;
			upsegAddQuery(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-I")==0) {
			i++;
			strcpy(indexfile,argv[i]);
			fprintf (stderr,"using index file '%s'\n",indexfile);
		}
		else if (strcmp(argv[i],"-i")==0) {
			i++;
			info.index = atoi(argv[i]);
			fprintf (stderr,"index set to %d\n",info.index);
		}
		else if (strcmp(argv[i],"-l")==0) {
			i++;
			strcpy(info.level,argv[i]);
			fprintf (stderr,"level set to %s\n",info.level);
		}
		else if (strcmp(argv[i],"-F")==0) {
			i++;
			info.first = atoi(argv[i]);
			fprintf (stderr,"first set to %d\n",info.first);
		}
		else if (strcmp(argv[i],"-L")==0) {
			i++;
			info.last = atoi(argv[i]);
			fprintf (stderr,"last set to %d\n",info.last);
		}
		else {
			if (argv[i][0]=='-') {
				fprintf (stderr,"wrong option '%s'!\n",argv[i]);
				fprintf (stderr,"use: %s unipen-file epsfile [options]!\n",argv[0]);
				print_options();
				exit(1);
			}
			else {
				upsegAddFile(&info,argv[i]);
			}
		}
	}
	info.current = 0;
	info.xmin = info.ymin = 0.0;
	info.xmax = info.ymax = 0.0;
	if (strcmp(epsfname,"tmp.eps")==0)
		fprintf (stderr,"using '%s' for output (default)\n",epsfname);
	return &info;
}

void update_min_max (upsegQueryInfo *info, sigSignal *sig)
{
	double xmin,xmax,ymin,ymax;
	double px,py,pz;
	int j;

	xmin = ymin = 9999999.0;
	xmax = ymax = -9999999.0;
	for (j=0;j<sig->nsamples;j++) {
		px = info->xscale*sig->x[j];
		py = info->yscale*sig->y[j];
		pz = sig->z[j];
		if (info->pu<1&&pz<=info->minpres)
			continue;
		if (px<xmin) xmin = px;
		if (px>xmax) xmax = px;
		if (py<ymin) ymin = py;
		if (py>ymax) ymax = py;
	}
	if ((xmax-xmin)>(info->xmax-info->xmin)) {
		info->xmin = xmin;
		info->xmax = xmax;
	}
	if ((ymax-ymin)>(info->ymax-info->ymin)) {
		info->ymin = ymin;
		info->ymax = ymax;
	}
}


#define XS(x) (xoffset+((x-xmin)/xrange-.5)*xfactor*w)
#define YS(y) (yoffset-((y-ymin)/yrange-.5)*yfactor*h)

void scale_points(int n, double *x, double *y,
	double xmin, double xmax, double ymin, double ymax,
	double w, double h, double xoffset, double yoffset, double margin)
{
	double xrange = xmax-xmin;
	double yrange = ymax-ymin;
	double xfactor = 1.;
	double yfactor = 1.;
	double height = h;
	int i;

/*
#define rescaled(a,r,f,min,offset,w) (offset + (((a)-min)/r-.5)*f*w)

        if(xrange <= 0.0) xrange = 1.0;
        if(yrange <= 0.0) yrange = 1.0;
        
	if (xrange > yrange) {
		yfactor = yrange/xrange;
        } else {
		xfactor = xrange/yrange;
	}
	
	w = w - 2. * margin * w;
	h = h - 2. * margin * h;

        if(w <= 0.0) w = 1.;
        if(h <= 0.0) h = 1.;

	for (i=0;i<n;i++) {
		new_val = rescaled(x[i],xrange,xfactor,xmin,xmax,xoffset,w);
		x[i] = new_val;
		new_val = rescaled(y[i],yrange,yfactor,ymin,ymax,yoffset,h);
		y[i] = new_val;
	}
*/

	w = w - 2. * margin * w;
	h = h - 2. * margin * h;
	
        if(w <= 0.0) w = 1.;
        if(h <= 0.0) h = 1.;
        
	if (xrange/w > yrange/h) {
		xfactor = 1.;
		yfactor = yrange/xrange;
		if(yfactor * w > h) {
			w = h;
		} else {
			h = w;
		}
	}
	else {
		xfactor = xrange/yrange;
		yfactor = 1.;
		if(xfactor * h > w) {
			h = w;
		} else {
			w = h;
		}
	}
	for (i=0;i<n;i++) {
	        if(isnan(x[i])) {
	            fprintf(stderr,"unipen2eps() points_x[%d]= isnan(%f) pre scale_points()\n", i,x[i]);
                    exit(1);
                }
                if(isnan(y[i])) {
                    fprintf(stderr,"unipen2eps() points_y[%d]= isnan(%f) pre scale_points()\n", i,y[i]);
                    exit(1);
                }

		x[i] = XS(x[i]);
		y[i] = height - YS(y[i]);

	        if(isnan(x[i])) {
	            fprintf(stderr,"unipen2eps() points_x[%d]= isnan(%f) post scale_points()\n", i,x[i]);
                    fprintf(stderr,"xoffset=%f xmin=%d xrange=%f xfactor=%f w=%f\n",xoffset,xmin,xrange,xfactor,w);
                    exit(1);
                }
                if(isnan(y[i])) {
                    fprintf(stderr,"unipen2eps() points_y[%d]= isnan(%f) post scale_points()\n", i,y[i]);
                    fprintf(stderr,"yoffset=%f ymin=%d yrange=%f yfactor=%f h=%f\n",yoffset,ymin,yrange,yfactor,h);
                    exit(1);
                }

	}

}
#define Y(y) (y)

/* scale points in a box */
void create_points (double points_x[MAX_SAMPLES],double points_y[MAX_SAMPLES]
	, sigSignal *s , int x, int y, upsegQueryInfo *info
	, double height, double boxwidth, double boxheight)
{
	double xmin,xmax,ymin,ymax;
	double xoffset,yoffset;
	int j;
	double xscale,yscale;

	if (s->nsamples>MAX_SAMPLES) {
		fprintf (stderr,"too many samples (%d) recompile with bigger MAX_SAMPLES (=%d)!\n",s->nsamples,MAX_SAMPLES);
		exit(1);
	}
	xscale = info->xscale;
	yscale = info->yscale;
	xoffset = (x+.5)*xscale*boxwidth;
	yoffset = .5*yscale*boxheight;

	points_x[0] = xscale*s->x[0];
	points_y[0] = yscale*s->y[0];
	xmin = ymin =  9999999.;
	xmax = ymax = -9999999.;
	for (j=0;j<s->nsamples;j++) {
		points_x[j] = xscale*s->x[j];
		points_y[j] = yscale*s->y[j];
		if (info->pu<1&&s->z[j] > info->minpres) {
  		  if (points_x[j]<xmin) xmin = points_x[j];
		  if (points_x[j]>xmax) xmax = points_x[j];
		  if (points_y[j]<ymin) ymin = points_y[j];
		  if (points_y[j]>ymax) ymax = points_y[j];
	        }
	        if(isnan(points_x[j])) {
	           fprintf(stderr,"unipen2eps() points_x[%d]= isnan(%f)\n", j,points_x[j]);
	           exit(1);
	        }
	        if(isnan(points_y[j])) {
	           fprintf(stderr,"unipen2eps() points_y[%d]= isnan(%f)\n", j,points_y[j]);
	           exit(1);
	        }
	}

	if (info->same_scale) {
		for (j=0;j<s->nsamples;j++) {
			points_x[j] = points_x[j]-xmin+info->xmin;
			points_y[j] = points_y[j]-ymin+info->ymin;
	                if(isnan(points_x[j])) {
	                   fprintf(stderr,"unipen2eps() points_x[%d]= isnan(%f) info->same_scale\n", j,points_x[j]);
	                   exit(1);
	                }
	                if(isnan(points_y[j])) {
	                   fprintf(stderr,"unipen2eps() points_y[%d]= isnan(%f) info->same_scale\n", j,points_y[j]);
	                   exit(1);
	                }
		}
		xmin = info->xmin;
		xmax = info->xmax;
		ymin = info->ymin;
		ymax = info->ymax;
		
	        fprintf(stderr,"xmin=%d xmax=%d ymin=%d ymax=%d\n", xmin,xmax,ymin,ymax);
	}

	scale_points(s->nsamples,points_x,points_y,xmin,xmax,ymin,ymax,
		boxwidth,boxheight,xoffset,yoffset,info->margin);

	xmin = xmax = points_x[0];
	points_y[0] = points_y[0] + (y-1.)*yscale*boxheight;
	ymin = ymax = points_y[0];
	for (j=1;j<s->nsamples;j++) {
#ifdef DEBUG
	        fprintf(stderr,"DBG points_y[j=%d]=%f y=%d yscale=%f boxheight=%f\n",j,points_y[j],y,yscale,boxheight);
#endif
		points_y[j] = points_y[j] + (y-1.)*yscale*boxheight;
	        if(isnan(points_x[j])) {
	           fprintf(stderr,"unipen2eps() points_x[%d]= isnan(%f) final scaling\n", j,points_x[j]);
	           exit(1);
	        }
	        if(isnan(points_y[j])) {
	           fprintf(stderr,"unipen2eps() points_y[%d]= isnan(%f) final scaling\n", j,points_y[j]);
	           exit(1);
	        }
		if (points_x[j]<xmin) xmin = points_x[j];
		if (points_y[j]<ymin) ymin = points_y[j];
		if (points_x[j]>xmax) xmax = points_x[j];
		if (points_y[j]>ymax) ymax = points_y[j];
	}
	fprintf (stderr,"BB is (%f,%f) (%f,%f)\n",xmin,ymin,xmax,ymax);
}

void administrate_eps (sigSignal *s , int x, int y, upsegQueryInfo *info
	, double height, double boxwidth, double boxheight)
{
	double xmin,xmax,ymin,ymax;
	double points_x[MAX_SAMPLES];
	double points_y[MAX_SAMPLES];

	create_points (points_x,points_y,s,x,y,info
		,height,boxwidth,boxheight);
}

void add_eps (FILE *fp, char *name, sigSignal *s , int x, int y
	, upsegQueryInfo *info, double height, double boxwidth, double boxheight)
{
	double x0,y0,x1,y1;
	double xmin,xmax,ymin,ymax;
	double points_x[MAX_SAMPLES];
	double points_y[MAX_SAMPLES];
	int j;
	double xoffset,yoffset;
	double z;
	double xscale,yscale;
	double last_x0sane, last_x1sane;

	xscale = info->xscale;
	yscale = info->yscale;
	xoffset = (x+.5)*xscale*boxwidth;
	yoffset = (y+.5)*yscale*boxheight;

	create_points (points_x,points_y,s,x,y,info
		,height,boxwidth,boxheight);

	if (!info->nolabel)
		fprintf (fp,"%f %f moveto (%s) show\n"
			,xoffset-.25*boxwidth+info->label_offsetx
			,yoffset-1.5*boxheight+info->label_offsety,name);

	x0 = points_x[0];
	y0 = points_y[0];
	z = s->z[0];
	last_x0sane = 0.0;
	last_x1sane = 0.0;
	
	if (info->pointsize>0)
		fprintf (fp,"%f %f moveto\n%f %f %d 0 360 arc fill\n",x0,y0,x0,y0,info->pointsize);
	for (j=1;j<s->nsamples;j++) {
		x1 = points_x[j];
		y1 = points_y[j];
	        if(isnan(x0)) {
	           fprintf(stderr,"unipen2eps() points x0[%d]= isnan(%f)\n", j,points_x[j]);
	           /* exit(1); */
	           x0 = last_x0sane; /* Hack to prevent NaN */
	        } else {
	           last_x0sane = x0;
	        }

	        if(isnan(x1)) {
	           fprintf(stderr,"unipen2eps() points x1[%d]= isnan(%f)\n", j,points_x[j]);
	           /* exit(1); */
	           x0 = last_x1sane; /* Hack to prevent NaN */
	        } else {
	           last_x1sane = x1;
	        }

		if (z<=info->minpres) {
			if (info->pu>0)
				fprintf (fp,"n %10.6f %10.6f m %10.6f %10.6f l d\n",x0,y0,x1,y1);
		}
		else {
			if (info->pd>0)
				fprintf (fp,"n %10.6f %10.6f m %10.6f %10.6f l s\n",x0,y0,x1,y1);
		}
		x0 = x1;
		y0 = y1;
		z = s->z[j];
		if (info->pointsize>0)
			fprintf (fp,"%f %f moveto\n%f %f %d 0 360 arc fill\n",x0,y0,x0,y0,info->pointsize);
	}
}

int main (int argc, char *argv[])
{
	char epsfname[512];
	FILE *fp,*fp_eps;
	int i,idx,nrows,ncols;
	double boxwidth,boxheight;
	int nsignals,NCOLS;
	sigSignal *sig,**signals;
	upsegQueryInfo *query_info;
	double width,height;
	double X0,Y0,X1,Y1;
	char **names;
	int nindices,*indices=NULL;

	indexfile[0] = '\0';
	query_info = parse_args(argc,argv,epsfname);
	signals = upsegGetSignals(query_info,&nsignals,&names);
	if (indexfile[0]=='\0') {
		indices = (int *) malloc (nsignals*sizeof(int));
		for (i=0;i<nsignals;i++)
			indices[i] = i;
		nindices = nsignals;
	} else {
		if ((fp=fopen(indexfile,"r"))==NULL) {
			fprintf (stderr,"unable to open indexfile '%s'!\n",indexfile);
			exit(1);
		}
		nindices = 0;
		while (fscanf(fp,"%d",&i)==1) {
			if (nindices==0)
				indices = (int *) malloc (sizeof(int));
			else
				indices = (int *) realloc (indices,(nindices+1)*sizeof(int));
			indices[nindices++] = i;
		}
		fclose(fp);
	}
	if ((fp_eps=fopen(epsfname,"w"))==NULL) {
		fprintf (stderr,"unable to open '%s' for output!\n",epsfname);
		exit(1);
	}

	if (nindices<=0) {
		fprintf (stderr,"no items fulfil your query!\n");
		exit(1);
	}
	for (i=0;i<nindices;i++) {
		idx = indices[i];
		fprintf (stderr,"  [%d] [%s] %d samples\n"
			,idx,names[idx],signals[idx]->nsamples);
	}

fprintf (stderr,"NCOLS=%d\n",query_info->ncolumns);
	if (query_info->ncolumns==-1) {
		i = (int) sqrt((double)nindices);
		ncols   = i;
		nrows   = (nindices+i-1)/i;
	}
	else {
		ncols   = NCOLS = query_info->ncolumns;
		nrows = (nindices+NCOLS-1)/NCOLS;
	}
	boxwidth = query_info->xscale*query_info->width/ncols;
	boxheight = query_info->yscale*query_info->height/nrows;
	width   = query_info->xscale*query_info->width;
	height  = query_info->yscale*query_info->height;
	fprintf (stderr,"taking %d = %dx%d allographs at %fx%f (%fx%f)\n"
		,nindices,ncols,nrows,width,height,boxwidth,boxheight);

	if (query_info->same_scale) {
		for (i=0;i<nindices;i++) {
			idx = indices[i];
			sig = signals[idx];
			update_min_max(query_info,sig);
		}
	}
/*
	X0 = Y0 = 9999999.0;
	X1 = Y1 = -9999999.0;
	for (i=0;i<nindices;i++) {
			sig = signals[indices[i]];
			administrate_eps (sig,i%ncols,i/ncols,query_info
				,height,boxwidth,boxheight);
	}
*/
        fprintf(stderr,"PS width=%f height=%f\n", width,height);
        
	fprintf (fp_eps,"%%!PS-Adobe-2.0 EPSF-2.0\n");
	fprintf (fp_eps,"%%%%BoundingBox: %f %f %f %f\n",0.0,0.0,width,height);

	fprintf (fp_eps,"/n { newpath } def\n");
	fprintf (fp_eps,"/m { moveto  } def\n");
	fprintf (fp_eps,"/l { lineto  } def\n");
	fprintf (fp_eps,"/s {\n");
	fprintf (fp_eps,"\t%f setlinewidth\n",query_info->pd);
	fprintf (fp_eps,"\t[] 0 setdash\n");
	fprintf (fp_eps,"\tstroke\n");
	fprintf (fp_eps,"} def\n");
	fprintf (fp_eps,"/d {\n");
	fprintf (fp_eps,"\t%f setlinewidth\n",query_info->pu);
	fprintf (fp_eps,"\t[1 3] 0 setdash\n");
	fprintf (fp_eps,"\tstroke\n");
	fprintf (fp_eps,"} def\n");

	fprintf (fp_eps,"\n/Times-Roman-Bold findfont %d scalefont setfont\n\n",query_info->fontsize);
	fprintf (fp_eps,"%d %d translate\n",query_info->page_offsetx,query_info->page_offsety);
	fflush(fp_eps);

	for (i=0;i<nindices;i++) {
			idx = indices[i];
			sig = signals[idx];
			fprintf (stderr,"adding segment %d (%s) %d samples at (%d,%d)\n",idx,names[idx],sig->nsamples,i%ncols,nrows-i/ncols);
			add_eps (fp_eps,names[idx],sig,i%ncols,nrows-i/ncols ,query_info,height,boxwidth,boxheight);
			fflush(fp_eps);
			sigDeleteSignal(sig);
	}
	fprintf (fp_eps,"showpage\n");
	fclose(fp_eps);
	return 0;
}
