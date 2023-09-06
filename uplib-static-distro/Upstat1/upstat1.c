/* upstat1.c
 * gets all UNIPEN .SEGMENT entries fulfilling a syntax
 * writes them out in a number of formats.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

#define _MAIN
#include "upstat1.h"
#include "allo_resample.h"

static int nmatching_entries_found = 0;

void print_options (void)
{
	int i;

	fprintf (stderr,"options are:\n");
	fprintf (stderr,"     -o  outfile (used as base for output)\n");
	fprintf (stderr,"     -p  patfile\n");
	fprintf (stderr,"     -s  string\n");
	fprintf (stderr,"     -i  index\n");
	fprintf (stderr,"     -l  level\n");
	fprintf (stderr,"     -F  first\n");
	fprintf (stderr,"     -L  last\n");
	fprintf (stderr,"     -Z  (for output in .COORD X Y Z format, default=X Y)\n");
	fprintf (stderr,"     -O  output-function\n");
	fprintf (stderr,"where output-function can be any of the following strings:\n");
	i = 0;
	while(output_function_names[i][0] != 0) {
		fprintf (stderr,"         %s\n",output_function_names[i]);
		++i;
	}
}

int parse_output_function (char *function_name)
{
	int i, n;

	i = 0;
	while(output_function_names[i][0] != 0) {
		if (strcmp(output_function_names[i],function_name)==0) {
	                fprintf (stderr,"-O: output_function '%s'=%d\n"
	                               ,function_name,i);
			return i;
		}
		++i;
	}
	n = i;
	fprintf (stderr,"unknown output_function '%s'!!\n",function_name);
	fprintf (stderr,"use one of: %d = %s\n",0,output_function_names[0]);
	for (i=1;i<n;i++)
		fprintf (stderr,"            %d = %s\n",i,output_function_names[i]);
	exit(1);
}

void initialize_output_function (OFunction_Info *oinfo)
{
	oinfo->output_function = _OUTPUT_UNIPEN;
	oinfo->width           = oinfo->height = 250;
	oinfo->brush           = 5;
	oinfo->margin          = 6;
	oinfo->m               = 30;
	oinfo->nbins           = 100;
	oinfo->nsmo            = 0;
	oinfo->dfact           = 1.0;
	oinfo->do_add_sincos   = 0;
	oinfo->do_add_phi      = 0;
	oinfo->use_Z           = 0;
	oinfo->range           = -1.;
	oinfo->norm_pos_size   = NORM_ALLO_POS_RADIUS;
	strcpy(oinfo->tag,"-tag?");
}

upsegQueryInfo *parse_args (int argc, char *argv[], char outfile[], OFunction_Info *oinfo)
{
	int i;
	static upsegQueryInfo info;

	if (argc<2) {
		fprintf (stderr,"use: %s unipen-file [unipen-files] [options]!\n",argv[0]);
		print_options();
		exit(1);
	}

	/* set default values */

	initialize_output_function(oinfo);
	upsegInitializeQueryInfo(&info);
	
        oinfo->output_function = -1;

	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-s")==0) {
			i++;
			upsegAddQuery(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-p")==0) {
			i++;
			upsegAddQueriesFromPatfile(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-O")==0) {
			i++;
			oinfo->output_function = parse_output_function(argv[i]);
		}
		else if (strcmp(argv[i],"-o")==0) {
			i++;
			strcpy(outfile,argv[i]);
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
		else if (strcmp(argv[i],"-m")==0) {
			i++;
			oinfo->margin = atoi(argv[i]);
			fprintf (stderr,"using margin = %d\n",oinfo->margin);
		}
		else if (strcmp(argv[i],"-n")==0) {
			i++;
			oinfo->m = atoi(argv[i]);
			fprintf (stderr,"using %d resample points\n",oinfo->m);
		}
		else if (strcmp(argv[i],"-nbins")==0) {
			i++;
			oinfo->nbins = atoi(argv[i]);
			fprintf (stderr,"using %d nbins\n",oinfo->nbins);
		}
		else if (strcmp(argv[i],"-nsmo")==0) {
			i++;
			oinfo->nsmo = atoi(argv[i]);
			fprintf (stderr,"using %d nsmo\n",oinfo->nsmo);
		}
		else if (strcmp(argv[i],"-d")==0) {
			i++;
			sscanf(argv[i],"%lf",&oinfo->dfact);
			fprintf (stderr,"using dfact=%f\n",oinfo->dfact);
		}
		else if (strcmp(argv[i],"-w")==0) {
			i++;
			oinfo->width = atoi(argv[i]);
			fprintf (stderr,"using width = %d\n",oinfo->width);
		}
		else if (strcmp(argv[i],"-tag")==0) {
			i++;
			strcpy(oinfo->tag, argv[i]);
			fprintf (stderr,"using -tag for output = %s\n",oinfo->tag);
		}
                else if (strcmp(argv[i],"-scale")==0) {		
                        i++;
                        if(i >= argc) {
                           
#define SCALMSG "Expecting -scale rawxy|leftxy|midxy|normalize[=default]\n"

                           fprintf(stderr,SCALMSG);
                           exit(1);
                        }
                        if (strcmp(argv[i],"normalize")==0) {
	                   oinfo->norm_pos_size   = NORM_ALLO_POS_RADIUS;
	                   fprintf (stderr,"-scale normalize to (0,0) and unit circle\n");
                        } else if (strcmp(argv[i],"rawxy")==0) {
	                   oinfo->norm_pos_size   = NORM_RAW;
	                   fprintf (stderr,"-scale rawxy: leave UNP coordinates unchanged in output (contour)\n");
	                } else if (strcmp(argv[i],"leftxy")==0) {
	                   oinfo->norm_pos_size   = NORM_LEFT;
	                   fprintf (stderr,"-scale leftxy\n");
	                } else if (strcmp(argv[i],"midxy")==0) {
	                   oinfo->norm_pos_size   = NORM_MIDDLE;
	                   fprintf (stderr,"-scale midxy\n");
	                } else {
                           fprintf(stderr,SCALMSG);
                           exit(1);
	                }
	        } 
		else if (strcmp(argv[i],"-r")==0) {
			i++;
			oinfo->range = atof(argv[i]);
			fprintf (stderr,"using range = %f in tablet coordinates for scaling\n",oinfo->range);
		}
		else if (strcmp(argv[i],"-Z")==0) {
			fprintf (stderr,"using .COORD X Y Z\n");
			oinfo->use_Z = 1;
		}
		else {
			if (argv[i][0]=='-') {
				fprintf (stderr,"wrong option '%s'!\n",argv[i]);
				fprintf (stderr,"use: %s unipen-file epsfile [options]!\n",argv[0]);
				print_options();
				exit(1);
			}
			else
				upsegAddFile(&info,argv[i]);
		}
	}
	if (oinfo->margin<oinfo->brush) {
		fprintf (stderr,"WARNING: margin %d must be >= brush %d (resetting)!!\n"
			,oinfo->margin,oinfo->brush);
		oinfo->margin = oinfo->brush;
	}
	return &info;
}

void init_output_function (tUPUnipen *pUnipen
	, FILE *fp_out, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
   int ifun;
   ifun = oinfo->output_function;
   
   if(ifun == _OUTPUT_UNIPEN) {
	init_output_unipen(pUnipen,fp_out,query_info,oinfo);

   } else if(ifun == _OUTPUT_NAMEDXY) {
        init_output_featxy(fp_out,query_info,oinfo);

   } else if(ifun == _OUTPUT_RAWXY) {
	init_output_featxy(fp_out,query_info,oinfo);

   } else if(ifun == _OUTPUT_XPDF
          || ifun == _OUTPUT_YPDF
          || ifun == _OUTPUT_WPDF
          || ifun == _OUTPUT_HPDF) {
	init_output_xpdf(fp_out,query_info,oinfo); 
   } else { 
	fprintf (stderr,"requested output function not implemented (yet)!\n");
	exit(1);
   }
}


void handle_potential_unipen_file (char *fname, FILE *fp_out
	, upsegQueryInfo *query_info, OFunction_Info *oinfo
	, XY_Chunk **xyobj, int *nobj)
{
	int i, ifun;
	tUPEntry **entries;
	sigCharStream **streams;
	int nentries;
	char **names;
	char **level_names;
	char *hierarchy;
	
	query_info->nfiles = 1;
	strcpy(query_info->files[0],fname);

	entries = upsegGetEntriesWithStreams (query_info,&nentries
	                                                ,&names
	                                                ,&streams
							,&level_names
							,&hierarchy);

	fprintf (stderr,"got %d entries in %s\n",nentries, fname);
	if (nentries<=0) {
		fprintf (stderr,"no items fulfil your query in '%s'!\n",fname);
		upDelUnipen(query_info->pUnipen);
		return;
	}
	nmatching_entries_found += nentries;

	init_output_function(query_info->pUnipen,fp_out,query_info,oinfo);

        ifun = oinfo->output_function;
	if(ifun == _OUTPUT_UNIPEN) {
		output_unipen(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);

	} else if(ifun ==  _OUTPUT_NAMEDXY) {
		output_featxy(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams,1);

	} else if(ifun ==  _OUTPUT_XPDF
	       || ifun ==  _OUTPUT_YPDF
	       || ifun ==  _OUTPUT_WPDF
	       || ifun ==  _OUTPUT_HPDF) {
		output_xpdf(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams,1
				,xyobj,nobj);

	} else if(ifun == _OUTPUT_RAWXY) {
		output_featxy(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
			,level_names,names,streams,0);

        } else {
                fprintf(stderr,"-O ? ifun=%d ?\n", ifun);
                exit(1);
	}

	for (i=0;i<nentries;i++) {
		free(level_names[i]);
		free(names[i]);
		sigDeleteCharStream(streams[i]);
		free(entries[i]->Entry);
		free(entries[i]);
	}
	free(streams);
	free(level_names);
	free(names);
	free(entries);
	free(hierarchy);
	upDelUnipen(query_info->pUnipen);
}

void walk_through_directories (FILE *fp_out, char *curpath, char *dirname
	, upsegQueryInfo *query_info, OFunction_Info *oinfo
	, XY_Chunk **xyobj, int *nobj)
{
	DIR *curdir;
	struct DIR_DEFINE *dptr;
	char newpath[512];

	if ((curdir=opendir(dirname))==NULL) {
		fprintf (stderr,"    [%s]\n",dirname);
		handle_potential_unipen_file (dirname,fp_out
		   ,query_info,oinfo,xyobj,nobj);
		return;
	}
	if (strcmp(curpath,"")!=0)
		sprintf (newpath,"%s/%s",curpath,dirname);
	else
		strcpy(newpath,dirname);
	if (chdir(newpath)!=0) {
		perror(dirname);
		closedir(curdir);
		return;
	}
	fprintf (stderr,"> %s\n",dirname);
	for (dptr=readdir(curdir);dptr!=NULL;dptr=readdir(curdir)) {
		if (dptr->d_name[0]!='.') {
			walk_through_directories(fp_out
			    ,newpath,dptr->d_name,query_info
			    ,oinfo,xyobj,nobj);
		}
	}
	closedir(curdir);
	if (strcmp(curpath,"")!=0)
		chdir(curpath);
}

void handle_directory (FILE *fp_out, char *path
      , upsegQueryInfo *query_info, OFunction_Info *oinfo
      , XY_Chunk **xyobj,int *nobj)
{
	char cwd[512];
	char here[512];

	getcwd(here,512);
	/* I need absolute pathnames, so go to the required directory (if it is no file) */
	if (chdir(path)!=0) { /* assuming it is a file */
		handle_potential_unipen_file (path,fp_out
		   ,query_info,oinfo,xyobj,nobj);
		return;
	}
	getcwd(cwd,512);
	walk_through_directories(fp_out,"",cwd,query_info
	                               ,oinfo,xyobj,nobj);
	chdir(here);
}

void printout_xyobj_lower(FILE *fp, XY_Chunk *xyobj, int nobj)
{
   int iobj;
   double yb;
   
   yb = xyobj[0].ybase;
   
   /* Lower rectangular contour, counterclockwise */
   
   for(iobj = 0; iobj < nobj; ++iobj) {
     /* YB_L */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmin, 0.0);
     /* LL */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmin, xyobj[iobj].ymax-yb);
     /* LR */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmax, xyobj[iobj].ymax-yb);
     /* YB_R */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmax, 0.0);
     /* Connect to next */
     if(iobj < nobj-1) {
        fprintf(fp,"%f %f\n", xyobj[iobj+1].xmin, 0.0);
     }
   }
}

void printout_xyobj_upper(FILE *fp, XY_Chunk *xyobj, int nobj)
{
   int iobj;
   double yb;
   
   yb = xyobj[0].ybase;
   
   /* Lower rectangular contour, counterclockwise */
   
   for(iobj = nobj-1; iobj >= 0; --iobj) {
     /* YB_R */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmax, 0.0);
     /* UR */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmax, xyobj[iobj].ymin-yb);
     /* UL */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmin, xyobj[iobj].ymin-yb);
     /* YB_L */
     fprintf(fp,"%f %f\n", xyobj[iobj].xmin, 0.0);
     /* Connect to previous */
     if(iobj >= 1 ) {
        fprintf(fp,"%f %f\n", xyobj[iobj-1].xmax, 0.0);
     }
   }
}

void printout_xyobj(XY_Chunk *xyobj, int nobj)
{
   FILE *fp;
   
   fp = fopen("xyobj.dat","w");

   printout_xyobj_lower(fp,xyobj,nobj);
   printout_xyobj_upper(fp,xyobj,nobj);
   
   fclose(fp);
}

int main(int argc, char *argv[])
{
   FILE *fp_out;
   upsegQueryInfo *query_info;
   OFunction_Info oinfo;
   char **dirnames;
   int i,ndirs,nobj;
   XY_Chunk *xyobj;
	
   xyobj = (XY_Chunk *) malloc(sizeof(XY_Chunk));
   if(xyobj == NULL) {
      fprintf(stderr,"Error mallox xyobj[]\n");
      exit(1);
   }
   nobj = 1;

   fprintf (stderr,"upstat1: %s\n",UR_VERSION);
   oinfo.outfile[0] = '\0';
	
   query_info = parse_args(argc,argv,oinfo.outfile,&oinfo);
   if (oinfo.outfile[0]=='\0') {
	fprintf (stderr,"upread: output to stdout\n");
	fp_out = stdout;
   } else if ((fp_out=fopen(oinfo.outfile,"r"))!=NULL) {
	fprintf (stderr,"outfile %s already exists!",oinfo.outfile);
	exit(1);
   } else if ((fp_out=fopen(oinfo.outfile,"w"))==NULL) {
	fprintf (stderr,"unable to open outfile %s!",oinfo.outfile);
	exit(1);
   }
   /* now handle all files and dirnames 
      in query_info as search-points */
	   
   ndirs = query_info->nfiles;
   dirnames = (char **) malloc (ndirs*sizeof(char *));
   for (i=0;i<ndirs;i++) {
	dirnames[i] = strdup(query_info->files[i]);
   }
	
   /* Main loop, dealing with all unipen input files */

   fprintf(stderr,"Processing %d file entries\n", ndirs);
   fflush(stdout);
   fflush(stderr);

   for (i=0;i<ndirs;i++) {
      handle_directory(fp_out,dirnames[i],query_info
                          ,&oinfo,&xyobj,&nobj);
   }
   /* nobj: one to much */
   --nobj;
   	
   printout_xyobj(xyobj,nobj);
        	
   /* Closing */

   fflush(stdout);
   fflush(stderr);

   if (fp_out!=NULL&&fp_out!=stdout) {
	fclose(fp_out);
   }
   fprintf (stderr,"found in total %d matching entries\n",nmatching_entries_found);
   fprintf (stderr,"Nchunks=%d featurized\n",nobj);
   return 0;
}
