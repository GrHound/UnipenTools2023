/* upread3.c
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

#include "output_featxy.h"
#include "upread3.h"
#include "output_image.h"
#include "writer_identifications.h"
#include "allo_resample.h"

static int nmatching_entries_found = 0;
static char *output_function_names[] = {
	"output_unipen",
	"output_featchar",
	"output_image",
	"output_namedxy",
	"output_rawxy",
	""
};

void usage (void)
{
	int i;

	fprintf (stderr,"options are:\n");
	fprintf (stderr,"     -o  outfile (used as base for 'output_image')\n");
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
        fprintf (stderr,"     -scale rawxy|leftxy|midxy|midxbasy|normalize[=default] coordinates\n");
	/* options for output_image and output_featchar */
	fprintf (stderr,"     -n  #resample points (for 'output_image' and 'output_featchar')\n");
	fprintf (stderr,"     -d  factor for adjusting resampling (between [0,1])\n");
	fprintf (stderr,"     -m  margin (for 'output_image')\n");
	fprintf (stderr,"     -b  brush  (for 'output_image')\n");
	fprintf (stderr,"     -w  width  (for 'output_image')\n");
	fprintf (stderr,"     -h  height (for 'output_image')\n");
	fprintf (stderr,"     -frmt [xbm|ppm|pgm] (for image output format\n");
	fprintf (stderr,"     -canvas imagefile.[pnm] (for background image of given size)\n");
	fprintf (stderr,"     -clearcanvas (with -canvas)\n");
	fprintf (stderr,"     -filledcontours (for pgmfeat coco contours)\n");
	fprintf (stderr,"     -nsmooth <i> (xy coordinate smoothing <= 1 is no filtering\n");
	fprintf (stderr,"     -inkmodel <i> (ink-deposition model, solid|ballpoint|fountain|fountainwetpaper|cal45|spray)\n");
	fprintf (stderr,"     -labeled  (image outfilename contains segname)\n");
}

int parse_output_function (char *function_name)
{
	int i, n;

	i = 0;
	while(output_function_names[i][0] != 0) {
		if (strcmp(output_function_names[i],function_name)==0) {
			return i;
		}
		++i;
	}
	n = i;
	fprintf (stderr,"unknown output_function '%s'!!\n",function_name);
	fprintf (stderr,"use one of: %d = %s\n",0,output_function_names[0]);
	for (i=1;i<n;i++)
		fprintf (stderr,"            %d = %s\n",0,output_function_names[0]);
	exit(1);
}

void initialize_output_function (OFunction_Info *oinfo)
{
	oinfo->output_function = _OUTPUT_UNIPEN;
	oinfo->width           = oinfo->height = 250;
	oinfo->brush           = 5;
	oinfo->margin          = 6;
	oinfo->mreq_nsamples   = 30;
	oinfo->dfact           = 1.0;
	oinfo->do_add_sincos   = 0;
	oinfo->do_add_phi      = 0;
	oinfo->use_Z           = 0;
	oinfo->range           = -1.;
	oinfo->norm_pos_size   = NORM_ALLO_POS_RADIUS;
	oinfo->canvas[0]       = NUL;
	oinfo->clearcanvas     = 0;
	oinfo->filledcontours  = 0;
	oinfo->nsmooth         = 0;
	oinfo->inkmodel        = INK_SOLID;
	oinfo->labeled_outfilnam = 0;
}

void expecting_arg(int iarg, int argc, char *argv[], char msg[])
{
   if(iarg >= argc) {
      fprintf(stderr,"FATAL: Expecting argument after %s (%s)\n", argv[iarg-1],msg);
      exit(1);
   } else {
      fprintf(stderr,"%s (%s)\n", argv[iarg-1],msg);
   }
}

void parse_inkmodel(int i,int argc,char *argv[],OFunction_Info *oinfo)
{
  if(strcmp(argv[i],"solid") == 0) {
    oinfo->inkmodel = INK_SOLID;
  } else if(strcmp(argv[i],"fountain") == 0) {
    oinfo->inkmodel = INK_FOUNTAIN;
  } else if(strcmp(argv[i],"ballpoint") == 0) {
    oinfo->inkmodel = INK_BALLPOINT;
  } else if(strcmp(argv[i],"fountainwetpaper") == 0) {
    oinfo->inkmodel = INK_FOUNTAINWETPAPER;
  } else if(strcmp(argv[i],"cal45") == 0) {
    oinfo->inkmodel = INK_CALLIGRAPHIC_45;
  } else if(strcmp(argv[i],"spray") == 0) {
    oinfo->inkmodel = INK_SPRAY_DISK_50;
  } else {
    fprintf(stderr,"-inkmode solid|ballpoint|fountain|cal45\n");
    exit(1);
  }
  fprintf (stderr,"-inkmodel %d\n", oinfo->inkmodel);
}			

upsegQueryInfo *parse_args (int argc, char *argv[], char outfile[], OFunction_Info *oinfo)
{
	int i;
	static upsegQueryInfo info;

	if (argc < 2) {
		fprintf (stderr,"Nargs<2? Use: %s unipen-file [unipen-files] [options]!\n",argv[0]);
		usage();
		exit(1);
	}

	/* set default values */

	initialize_output_function(oinfo);
	upsegInitializeQueryInfo(&info);
	
	info.index = -1;

	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-s")==0) {
			i++;
			expecting_arg(i,argc,argv,"key for Unipen chunk selection");
			upsegAddQuery(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-p")==0) {
			i++;
			expecting_arg(i,argc,argv,"patterns for Unipen chunk selection");
			upsegAddQueriesFromPatfile(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-O")==0) {
			i++;
			expecting_arg(i,argc,argv,"output function");
			oinfo->output_function = parse_output_function(argv[i]);
			fprintf(stderr,"-O %s\n", argv[i]);
		}
		else if (strcmp(argv[i],"-o")==0) {
			i++;
			expecting_arg(i,argc,argv,"output file or image file prefix");
			strcpy(outfile,argv[i]);
		}
		else if (strcmp(argv[i],"-canvas")==0) {
			i++;
			expecting_arg(i,argc,argv,"canvas image on which to paint trajectory");
			strcpy(oinfo->canvas,argv[i]);
		}
		else if (strcmp(argv[i],"-clearcanvas")==0) {
			fprintf (stderr,"-clearcanvas (clear canvas for each entry)\n");
			oinfo->clearcanvas = 1;
		}
		else if (strcmp(argv[i],"-filledcontours")==0) {
			fprintf (stderr,"-filledcontours (assume closed contours which are to be filled with a color)\n");
			oinfo->filledcontours = 1;
		}
		else if (strcmp(argv[i],"-inkmodel")==0) {
		        i++;
			expecting_arg(i,argc,argv,"0=solid,1=ballpoint,2=fountainwetpaper");
			parse_inkmodel(i,argc,argv,oinfo);
		}
		else if (strcmp(argv[i],"-labeled")==0) {
			fprintf (stderr,"-labeled (image outfilnam contains segname)\n");
			oinfo->labeled_outfilnam = 1;
		}
		else if (strcmp(argv[i],"-nsmooth")==0) {
			i++;
			expecting_arg(i,argc,argv,"lowpass filtering of xy coordinates");
			oinfo->nsmooth = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-i")==0) {
			i++;
			expecting_arg(i,argc,argv,"index of Unipen entry");
			info.index = atoi(argv[i]);
			fprintf (stderr,"-i %s -> specifically extracting Unipen segment with index %d\n"
			               ,argv[i],info.index);
		}
		else if (strcmp(argv[i],"-l")==0) {
			i++;
			expecting_arg(i,argc,argv,"level code of Unipen .HIERARCHY");
			strcpy(info.level,argv[i]);
			fprintf (stderr,"level set to %s\n",info.level);
		}
		else if (strcmp(argv[i],"-F")==0) {
			i++;
			expecting_arg(i,argc,argv,"first entry in Unipen file");
			info.first = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-L")==0) {
			i++;
			expecting_arg(i,argc,argv,"last entry in Unipen file");
			info.last = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-m")==0) {
			i++;
			expecting_arg(i,argc,argv,"margin of image");
			oinfo->margin = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-n")==0) {
			i++;
			expecting_arg(i,argc,argv,"number of time-resampled points");
			oinfo->mreq_nsamples = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-d")==0) {
			i++;
			expecting_arg(i,argc,argv,"delta in sampling");
			sscanf(argv[i],"%lf",&oinfo->dfact);
		}
		else if (strcmp(argv[i],"-w")==0) {
			i++;
			expecting_arg(i,argc,argv,"requested image width in pixels");
			oinfo->width = atoi(argv[i]);
		}
                else if (strcmp(argv[i],"-scale")==0) {		
                        i++;
                        if(i >= argc) {
                           
#define SCALMSG "Expecting -scale rawxy|leftxy|midxy|midxbasy|normalize[=default]\n"

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
	                } else if (strcmp(argv[i],"expand")==0) {
	                   oinfo->norm_pos_size   = NORM_EXPAND;
	                   fprintf (stderr,"-scale expand\n");

	                } else if (strcmp(argv[i],"midxbasy")==0) {
	                   oinfo->norm_pos_size   = NORM_MID_X_BASELINE_Y;
	                   fprintf (stderr,"-scale midxbasy\n");
	                } else {
                           fprintf(stderr,SCALMSG);
                           exit(1);
	                }

	        } 
		else if (strcmp(argv[i],"-frmt")==0) {
			i++;
			if (strcmp(argv[i],"xbm")==0) {
				oinfo->im_frmt = O_XBM;
				fprintf (stderr,"-frmt %s using image output in XBM format\n",argv[i]);
			} else if (strcmp(argv[i],"ppm")==0) {
				oinfo->im_frmt = O_PPM;
				fprintf (stderr,"-frmt %s using image output in PPM format\n", argv[i]);
			} else if (strcmp(argv[i],"pgm")==0) {
				oinfo->im_frmt = O_PGM;
				fprintf (stderr,"-frmt %s using image output in PGM format\n",argv[i]);
			} else {
				fprintf (stderr,"unknown image output-format '%s'!\n",argv[i]);
				fprintf (stderr,"use one of 'xbm' 'ppm' or 'pgm'\n");
				usage();
				exit(1);
			}
		}
		else if (strcmp(argv[i],"-h")==0) {
			i++;
			expecting_arg(i,argc,argv,"requested image height in pixels");
			oinfo->height = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-b")==0) {
			i++;
			expecting_arg(i,argc,argv,"requested brush width in pixels");
			oinfo->brush = atoi(argv[i]);
			fprintf (stderr,"-b %d (requested brush width in pixels)\n",oinfo->brush);
		}
		else if (strcmp(argv[i],"-r")==0) {
			i++;
			expecting_arg(i,argc,argv,"requested range of tablet coordinates");
			oinfo->range = atof(argv[i]);
		}
		else if (strcmp(argv[i],"-Z")==0) {
			fprintf (stderr,"-Z (using .COORD X Y Z)\n");
			oinfo->use_Z = 1;
		}
		else {
			if (argv[i][0]=='-') {
				fprintf (stderr,"wrong option '%s'!\n",argv[i]);
				fprintf (stderr,"use: %s unipen-file epsfile [options]!\n",argv[0]);
				usage();
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
	switch (oinfo->output_function) {
		case _OUTPUT_UNIPEN:
			init_output_unipen(pUnipen,fp_out,query_info,oinfo);
			break;
		case _OUTPUT_FEATCHAR:
			init_output_featchar(fp_out,query_info,oinfo);
			break;
		case _OUTPUT_IMAGE:
			break;
		case _OUTPUT_NAMEDXY:
			init_output_featxy(fp_out,query_info,oinfo);
			break;
		case _OUTPUT_RAWXY:
			init_output_featxy(fp_out,query_info,oinfo);
			break;
		default:
			fprintf (stderr,"requested output function not implemented (yet)!\n");
			exit(1);
	}
}


void handle_potential_unipen_file (char *fname, FILE *fp_out
	, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	int i;
	tUPEntry **entries;
	sigCharStream **streams;
	int nentries;
	char **names;
	char **level_names;
	char *hierarchy;
	Yfont yfont;
	
	query_info->nfiles = 1;
	strcpy(query_info->files[0],fname);
	oinfo->writer_code = updirFilename2WriterCode(fname);
	updirectFname_to_writer_id(fname, oinfo->writer_codestr);

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

	switch (oinfo->output_function) {
		case _OUTPUT_UNIPEN:
			output_unipen(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		case _OUTPUT_FEATCHAR:
			output_featchar(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		case _OUTPUT_IMAGE:
			output_image(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		case _OUTPUT_NAMEDXY:
			output_featxy(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams,1,&yfont);
			break;
		case _OUTPUT_RAWXY:
			output_featxy(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams,0,&yfont);
			break;
		default:
			break;
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
	, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	DIR *curdir;
	struct DIR_DEFINE *dptr;
	char newpath[512];

	if ((curdir=opendir(dirname))==NULL) {
		fprintf (stderr,"    [%s]\n",dirname);
		handle_potential_unipen_file (dirname,fp_out,query_info,oinfo);
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
			walk_through_directories(fp_out,newpath,dptr->d_name,query_info,oinfo);
		}
	}
	closedir(curdir);
	if (strcmp(curpath,"")!=0)
		chdir(curpath);
}

void handle_directory (FILE *fp_out, char *path, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	char cwd[512];
	char here[512];

	getcwd(here,512);
	/* I need absolute pathnames, so go to the required directory (if it is no file) */
	if (chdir(path)!=0) { /* assuming it is a file */
		handle_potential_unipen_file (path,fp_out,query_info,oinfo);
		return;
	}
	getcwd(cwd,512);
	walk_through_directories(fp_out,"",cwd,query_info,oinfo);
	chdir(here);
}

int main(int argc, char *argv[])
{
	FILE *fp_out;
	upsegQueryInfo *query_info;
	OFunction_Info oinfo;
	char **dirnames;
	int i,ndirs;

	fprintf (stderr,"upread3: %s\n",UR_VERSION);
	oinfo.outfile[0] = '\0';
	
	query_info = parse_args(argc,argv,oinfo.outfile,&oinfo);
	if (oinfo.output_function!=_OUTPUT_IMAGE) {
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
	} else
		fp_out = NULL;
/*
	read_writer_identifications(WRITER_ID_FILE);
*/
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
		handle_directory(fp_out,dirnames[i],query_info,&oinfo);
	}
	
	/* Closing */
	
	fflush(stdout);
	fflush(stderr);

	if (fp_out!=NULL&&fp_out!=stdout) {
		fclose(fp_out);
	}
	fprintf (stderr,"found in total %d matching entries\n",nmatching_entries_found);
	return 0;
}
