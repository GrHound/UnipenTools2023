/**************************************************************************
*                                                                         *
*  UNIPEN PROJECT (upview.c)                                              *
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
/*
     Program upview
     
     Purpose: browser for Unipen files on Unix X stations.
     
     L.R.B. Schomaker / G.H. Abbink / L.G. Vuurpijl
    
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "uplib.h"
#include "upview.h"
#include "upsiglib.h"

float minQ = -1.0;
float maxQ = -1.0;

	void	error();
	int	nfields();
	int	scalesam();
	int	scalesam_real();
	int	scalesam_auto();
	int	readchunks();
	void	draw_chunkbox();
	void	flash_chunkbox();
	void	flash_controlbox();
	void	drawall();
	int	handle_buttonpress();
	int     chunkboxx();
	int     chunkboxy();
	void    set_layout();
	void    setup_boxes();
	void    parse_args();
	void    print_usage();
	void    draw_line();
	void    draw_dot();
	int     nint4();
	int     update_unipenbox();
	void    clear_unipenbox();
	void    scale_sample_real();
	void    scale_sample_auto();
	void    scale_sample();
	void    bound_y_offset();
	void    handle_word_clicked();
        char    *get_correct_label();
        char    *make_stripped_label();
        int     igetfilesize();
		  int     file_has_changed();
        void    finish_file();
        void    mark_boxes();
        
        void open_postscript();
        void close_postscript();

/*****/
/* a module for scaling, required are .X_DIM, .Y_DIM, .H_LINE if Tablet Scale is present */

void get_scale_box_parameters (tUPUnipen   *_pUnipen_, Plotcontrol *pctl)
/* is called for getting box-parameters for SCALE_REAL_SETUP */
{
   register int i;
   char *char_ptr, *ptr_offset;
   char *XDim, *YDim, *HLine;

	pctl->x_dim_given  = 0;
	pctl->y_dim_given  = 0;
	pctl->h_line_given = 0;
        
   if ((XDim = upGetArgument (_pUnipen_, cKeyXDim, 0)) != NULL)
   {
      pctl->x_dim = strtol(&XDim [strlen (cKeyXDim)], &char_ptr, 10);
      pctl->x_dim_given = 1;
   } else {
		pctl->scale_mode = SCALE_PER_CHUNK;
		return;
	}

   if ((YDim = upGetArgument (_pUnipen_, cKeyYDim, 0)) != NULL)
   {
      pctl->y_dim = strtol(&YDim [strlen (cKeyYDim)], &char_ptr, 10);
      pctl->y_dim_given = 1;
   } else {
		pctl->scale_mode = SCALE_PER_CHUNK;
		return;
	}

   if ((HLine = upGetArgument (_pUnipen_, cKeyHLINE, 0)) != NULL)
   { 
      ptr_offset = HLine + strlen (cKeyHLINE);
      i = 0;
      while(i <= MAX_HLINES) {
         char_ptr = ptr_offset;
         pctl->h_lines[i] = strtol(ptr_offset, &char_ptr, 10);
         pctl->h_line_given = 1;
         if(char_ptr != ptr_offset) {
            ptr_offset = char_ptr;
            ++i;
         } else {
            break;
         }
      }
      pctl->h_line_nlines = i;
      
   } else {
		pctl->scale_mode = SCALE_PER_CHUNK;
		return;
	}
} /* End get_scale_box_parameters() */ 

int was_scale_request (char *descr, Plotcontrol *pctl)
{ 
	if (strcmp(descr, LAB_SCALE_PER_CHUNK) == 0) {
         pctl->scale_mode = SCALE_ON_BIGGEST;
			return 1;

   } else if ( strcmp(descr, LAB_SCALE_ON_BIGGEST) == 0) {
      if ( pctl->x_dim_given
            && pctl->y_dim_given
            && pctl->h_line_given) {
         pctl->scale_mode = SCALE_REAL_SETUP;
      } else {
         pctl->scale_mode = SCALE_PER_CHUNK;
      }
      return 1;
   } else if (strcmp(descr, LAB_SCALE_REAL_SETUP) == 0) {
       pctl->scale_mode = SCALE_PER_CHUNK;
       return 1;
   }
   return 0;
}

int readsamples (tUPUnipen   *_pUnipen_, tUPEntry *entry, char filename[]
	,Chunktype chunk[MAXCHUNKS]
	,int       nchunk
	,char      level_names[]
	,int       level
	,int       level_max
	,int       scale_mode
	,float     qual
	,int       use_labels)
{
   int i;
   double dx, dy;
   sigSignal *usignal;

/*
   if (scale_mode == SCALE_REAL_SETUP) {
      	 if (!update_unipenbox (_pUnipen_, &chunk [nchunk]))
			 	return 0;
   } else {
      	 clear_unipenbox (&chunk [nchunk]);
   }
*/    
/*
   usignal = sigEntry2Signal(_pUnipen_,entry,RAW_SIGNAL);
*/
   usignal = sigEntry2Signal(_pUnipen_,entry,TIME_EQUI_DIST);
   chunk [nchunk].signal_stream = usignal;
   chunk [nchunk].va = (int *) malloc (usignal->nsamples * sizeof (int));

   sprintf(chunk[nchunk].file_name,"File=%s Hierarchy=%s"
                                              , filename, level_names);
   chunk[nchunk].lfile_name = strlen(chunk[nchunk].file_name);
	if (use_labels) {
	if (qual!=-1.0) {
   	sprintf(chunk[nchunk].correct_name,"%s(%.2f)"
			, get_correct_label (upGetArgument (_pUnipen_, ".SEGMENT", 1)
				,  _pUnipen_->Levels[level]),qual);
	} else {
   	strcpy(chunk[nchunk].correct_name
			, get_correct_label (upGetArgument (_pUnipen_, ".SEGMENT", 1)
				,  _pUnipen_->Levels[level]));
	}

   chunk[nchunk].lcorrect_name = strlen(chunk[nchunk].correct_name);
	}
   /* nr of samples */
   chunk[nchunk].nxyz = usignal->nsamples;
    
   /* Calculate first-order difference "absolute velocity" to reveal sampling characteristics */
   chunk[nchunk].va[0] = 0;

   for (i = 1; i < chunk[nchunk].nxyz; i++)
   {
	 dx = usignal->x[i] - usignal->x[i-1];
	 dy = usignal->y[i] - usignal->y[i-1];
	 chunk[nchunk].va[i] = (int) (sqrt(dx*dx + dy*dy));
   }

   chunk[nchunk].is_drawn  = 0;
   chunk[nchunk].is_scaled = 0;
	return 1;
} /* End readsamples() */

/* event handling changed by Loe to add polling nov '97*/
#define FileChanged 12345
int Loe_getevent(fname,do_poll,event)
char *fname;
int do_poll;
XEvent *event;
{     
	struct timeval timeout;

	if (do_poll) {
		while (1) {
			if (XCheckMaskEvent(dpy,ExposureMask|ButtonPressMask|ButtonReleaseMask,event)==True) {
				return(event->type);
			} else {
				if (file_has_changed(fname)) {
					return FileChanged;
				}
			}
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
			select(0,NULL,NULL,NULL,&timeout);
		}
	} else {
		XNextEvent(dpy, event);
		return(event->type);
	}
}

/*****/
int main (int argc, char *argv[])
{
	int nchunk, minpres = 20, quit_request, iarg;
	int level_requested, level, level_max;
	int ncols_requested, y_offset = 0;
	int nbytes_in_file;
	static int first_arg = 0;
	Chunktype *chunk;
	tUPEntry *entry;
	Plotcontrol   pctl;
	XEvent	ev; /* X11 */
	tUPUnipen *pUnipen; /* unipen */
	char unipen_definition_file[100];
	static Boxtype	boxes[NBOXES];
	static Layout layout;
	static char  wheader[NCHARMAX];
	static char  filetitle[NCHARMAX];
	char level_names[NCHARMAX];
	int StateFileIO;
	int ChangeFile = 0;
	int QuitLoop = 0;
	int Index;
	int ScaleWidth, ScaleHeight, ScaleMaxT, ScaleMaxV;

	int do_redraw_on_button_press = 0;

	int do_poll; /* added by Loe, based on xv idea... Nov 1997 */

	float qual;

#ifndef UV_VERSION
#define UV_VERSION "V3.03 March 1997 - L.Schomaker, L.Vuurpijl, G.Abbink  - NICI"
#endif

	printf("upview %s The Netherlands\n",UV_VERSION);
   
	if (argc < 2) {
		print_usage();
		exit(1);
	}
 
	chunk = (Chunktype *) malloc(MAXCHUNKS * sizeof(Chunktype));
	ZERO(chunk);

	parse_args(argc, argv
		, &minpres                 /* Pressure criterion in case of P */
		, &ncols_requested         /* Number of columns on screen */
		, &pctl.scale_mode         /* AutoScale per box or per file */
		, &pctl.y_flip             /* Flip y coordinates */
		, &pctl.veloc              /* Show (pseudo)velocity */
		, &pctl.nthick             /* Line thickness, neg. is gesture */
		, &level_requested         /* Requested hierarchy level number */
		, &pctl.gesture_mode       /* Draw thick start, thin tail */
		, &pctl.show_penup         /* Show penup streams too */
		, &first_arg               /* Number of first UNIPEN file argv */
		, &do_poll                 /* lets upview poll for a file */
		, &pctl.no_labels              /* upview displays no names of segments */
		, unipen_definition_file);  

fprintf (stderr,"NL + %d\n",pctl.no_labels);

	/* postscript initializations */
	pctl.Do_PostScript = 0;
	pctl.N_PostScript = 1;
	pctl.Do_PostScript_BoxTitle = pctl.gesture_mode;
	if(pctl.gesture_mode)
		if(pctl.nthick == 1)
			pctl.nthick = INITIAL_GESTURE_THICKNESS;
	pctl.open_lines = 0;
	pctl.rounded_lines = 0;

	/* other initializations */
	sprintf(wheader,"UniPen Viewer %s %s",argv[0],UV_VERSION);
	X_initialize(WINDW, WINDH, 10, 10, wheader);
	pUnipen = upNewUnipen (unipen_definition_file);
	if (pUnipen==NULL)
		exit(1);
	iarg = first_arg;
	mark_boxes(boxes, cBoxStateChanged);

	/* XSelectInput: added by Loe to enable polling (nov 1997) */
	XSelectInput (dpy,window1,ExposureMask|ButtonPressMask|ButtonReleaseMask);

	while (!QuitLoop) {
		quit_request = 0; /* reset quit request */
		ChangeFile = 0;   /* reset change file */
		nchunk = 0;       /* reset nchunk */
		level_max = 0;    /* reset level_max */
		ScaleWidth  = 0;  /* reset scaling */
		ScaleHeight = 0;
		ScaleMaxT   = 0;
		ScaleMaxV   = 0;
     
		set_layout(ncols_requested,&layout); 
		level = level_requested; 
		StateFileIO = cStateFileToBeOpened;

		while (!ChangeFile) {
			XFlush (dpy);

			/* detect idle time */
			if ((quit_request == 0) && ((StateFileIO == cStateFileIsClosed) 
				|| (XEventsQueued (dpy, QueuedAfterReading) != 0))) {

				Dprintf (stderr, "EEN, quit: %d\n", quit_request);

				switch (Loe_getevent (argv[iarg],do_poll,&ev)) { /* event available */

					case FileChanged:
						StateFileIO = cStateFileToBeOpened;
						goto retry_with_next_file;
						break;

					case ButtonRelease: ;
						if (do_redraw_on_button_press) {
							Dprintf (stderr, "Drawall because of ButtonRelease\n");
							/* mark_boxes(boxes, cBoxStateChanged); */
							drawall (&ev, 1, chunk, nchunk, boxes
								, minpres, &layout, y_offset, &pctl); 
						}
						break;

					case Expose:

						Dprintf (stderr, "Drawall because of expose\n");
						drawall (&ev, 2, chunk, nchunk, boxes, minpres, &layout, y_offset, &pctl); 
						break;

					case ButtonPress:

						if (quit_request == 0) { 
							Dprintf (stderr, "ButtonPress\n");
							Dflush (stderr);
							if (handle_buttonpress (pUnipen, &ev, boxes, chunk, nchunk
								, &quit_request
								, &y_offset
								, &pctl
								, &ncols_requested
								, &level_requested
								, level_max
								, &layout
								, argv[iarg]
								, StateFileIO)) {
								Dprintf (stderr, "!! ButtonPress (quit = %d)\n", quit_request);
								Dflush (stderr);
								if (quit_request == USR_ZOOM) {
									set_layout(ncols_requested,&layout); 
									setup_boxes(boxes, chunk, nchunk, &layout, level_max, &pctl);
									scalesam(1, &ScaleWidth, &ScaleHeight, &ScaleMaxT, &ScaleMaxV, boxes, chunk, nchunk, minpres, &layout, &pctl); 
									bound_y_offset(&y_offset,boxes,NCONTROLBOXES+nchunk-1);
									quit_request = 0;
								}
								do_redraw_on_button_press = 1;
							} else
								do_redraw_on_button_press = 0;

						}
						break;
				
				} /* endswitch X_getevent() */

			} else {

				Dprintf (stderr, "TWEE, quit: %d\n", quit_request);
				/* quit request */
				switch (quit_request) {

					case USR_QUIT:

						Dprintf (stderr, ":::: QUIT ::::\n");
						Dflush (stderr);
						switch (StateFileIO) {
							case cStateFileIsClosed:
								StateFileIO = cStateFileToBeFreed;
								break;
							case cStateFileToBeFreed:
								/* do NOT remove this */
								break;
							case cStateFileIsFreed:
								QuitLoop = 1;
								ChangeFile = 1;
								break;
							default:
								StateFileIO = cStateFileToBeClosed;
								break;
						}
						break;

					case USR_PREVF:

						Dprintf (stderr, ":::: PREV FILE ::::\n");
						Dflush (stderr);
						if (iarg - 1 < first_arg) {
							XBell (dpy, 100);
							quit_request = 0; /* reset quit request */
						}
						else {
							switch (StateFileIO) {
								case cStateFileIsClosed:
									StateFileIO = cStateFileToBeFreed;
									break;
								case cStateFileToBeFreed:
									/* do NOT remove this */
									break;
								case cStateFileIsFreed:
									iarg = iarg - 1;
									nbytes_in_file = igetfilesize(argv[iarg]);
									sprintf(filetitle,"(Reading %s, %d bytes)"
										, argv[iarg], nbytes_in_file);
									/* Draw reading of file name */
									XClearArea (dpy,window1,0,0,window_paint,Y_TITLE_AREA/2,False);
									XDrawImageString(dpy, window1, gcFileName, BUTD, BUTD  
										,filetitle,strlen(filetitle));
									ChangeFile = 1;
									break;
								default:
									StateFileIO = cStateFileToBeClosed;
									break;
							}
						}
						break;

					case USR_NEXTF:

						Dprintf (stderr, ":::: NEXT FILE ::::\n");
						Dflush (stderr);
						if (iarg + 1 >= argc) {
							XBell (dpy, 100);
							/* reset quit request */
							quit_request = 0;
						}
						else {
							switch (StateFileIO) {
								case cStateFileIsClosed:
									StateFileIO = cStateFileToBeFreed;
									break;
								case cStateFileToBeFreed:
									/* do NOT remove this */
									break;
								case cStateFileIsFreed:
									iarg = iarg + 1;
									nbytes_in_file = igetfilesize(argv[iarg]);
									sprintf(filetitle,"(Reading %s, %d bytes)"
										, argv[iarg], nbytes_in_file);
									/* Draw reading of file name */
									XClearArea (dpy,window1,0,0,window_paint,Y_TITLE_AREA/2,False);
									XDrawImageString(dpy, window1, gcFileName, BUTD, BUTD  
										,filetitle,strlen(filetitle));
									ChangeFile = 1;
									break;
								default:
									StateFileIO = cStateFileToBeClosed;
									break;
							}
						}
						break;

					case USR_ZOOM:

						/* reset user request */
						quit_request = 0;
						break;

					case USR_HIERARCHY:

						level = level_requested;
						Dprintf (stderr, ":::: THIS FILE ::::\n");
						Dflush (stderr);
						switch (StateFileIO) {
							case cStateFileIsClosed:
								StateFileIO = cStateFileToBeFreed;
								break;
							case cStateFileToBeFreed:
								/* do NOT remove this */
								break;
							case cStateFileIsFreed:
								ChangeFile = 1;
								break;
							default:
								StateFileIO = cStateFileToBeClosed;
								break;
						}
						break;
				} /* endswitch quit_request */

				/* state file io */
         
				switch (StateFileIO) {

           case cStateFileIsClosed:

						/* nothing to do */
						break;

					case cStateFileToBeOpened:

						retry_with_next_file:

						Dprintf (stderr, "==== File %s is to be opened ====\n", argv [iarg]);
						Dflush (stderr);
						strcpy(pctl.Title_PostScript, argv[iarg]);
						if (upNextFile(pUnipen,argv[iarg]) && (pUnipen->NrOfLevels != 0)) {


   get_scale_box_parameters (pUnipen,&pctl);

							/* update state fileIO */
							StateFileIO = cStateFileIsOpen;
							/* levels */
							level_max = pUnipen->NrOfLevels - 1;
							if (level_requested < 0)
								level_requested = 0;
							if (level_requested > level_max)
								level_requested = level_max;
							level = level_requested;
							/* level names */
							strcpy (level_names,"");
 
							for (Index = 0; Index < pUnipen->NrOfLevels; ++Index ) {
								if (Index == level) {
									strcat (level_names,"[");
									strcat (level_names, pUnipen->Levels [Index]);
									strcat (level_names,"] ");
								}
								else {
									strcat (level_names, pUnipen->Levels [Index]);
									strcat (level_names," ");
								}
							}

							/* init chunks */
							nchunk = 0;
							/* etc. */
							y_offset = 0;
							quit_request = 0;
							/* setup boxes */
							setup_boxes(boxes, chunk, nchunk, &layout, level_max, &pctl);
							Dprintf (stderr, "Drawall because of file open\n");
							sprintf(chunk[nchunk].file_name,"File=%s Hierarchy=%s"
								, argv[iarg], level_names);
							chunk[nchunk].lfile_name = strlen(chunk[nchunk].file_name);

							drawall (&ev, 2, chunk, nchunk, boxes, minpres, &layout, y_offset, &pctl); 
						
						}
						else {
							if (do_poll) {
								fprintf (stderr,"File [%s] not ready yet, retrying.....\n",argv[iarg]);
								goto retry_with_next_file;
							}
							/* update state fileIO */
							fprintf(stderr,"\n UNIPEN input data file [%s] cannot be opened correctly\n", argv[iarg]);
							fprintf (stderr,"Maybe the file does not exist, or ");
							fprintf (stderr,"probably it is no UNIPEN file???\n");
							if (iarg>=argc-1)
								exit(1);
							iarg++;
/*							StateFileIO = cStateFileToBeOpened; */ /* This is an understatement (LS) */
							goto retry_with_next_file;
						}
						break;

					case cStateFileToBeClosed:

						Dprintf (stderr, "==== File is to be closed ====\n");
						Dflush (stderr);
						/* update state */
						StateFileIO = cStateFileIsClosed;
						/* done file */
						upDoneFile (pUnipen);
						break;

					case cStateFileIsOpen:

						Dprintf (stderr, "==== File is open ====\n");
						Dflush (stderr);
						if ((entry=upNextSegment (pUnipen,pUnipen->Levels[level],1))!=NULL) {
							Dprintf (stderr, "==== Chunk %d is to be read ====\n", nchunk);
							Dflush(stderr);
							qual = -1.0;
							if (strncmp(pUnipen->Levels[level],"CHAR",4)==0) {
								qual = upEntryFloatQuality(entry);
								if (qual!=-1.0) {
									if (minQ!=-1 && minQ>qual)
										break;
									if (maxQ!=-1 && maxQ<qual)
										break;
								}
							}
							if (!readsamples(pUnipen, entry, argv[iarg], chunk, nchunk
								, level_names, level, level_max, pctl.scale_mode,qual
								, pctl.no_labels==0)) {
								continue;
							}
							nchunk = nchunk + 1;
							if (nchunk >= MAXCHUNKS) {
								/* update state fileIO */
								StateFileIO = cStateFileToBeClosed;
								printf("Max chunk %d reached\n", MAXCHUNKS);
							}
							/* etc. */
							setup_boxes (boxes, chunk, nchunk, &layout, level_max, &pctl);
							Dprintf (stderr, "==== Until nChunks=%d: scale & draw if needed ====\n", nchunk);
							Dflush(stderr);
							if (scalesam (0, &ScaleWidth, &ScaleHeight, &ScaleMaxT, &ScaleMaxV, boxes, chunk, nchunk, minpres, &layout, &pctl)) {
								/* scale all chunks */
								scalesam (1, &ScaleWidth, &ScaleHeight, &ScaleMaxT, &ScaleMaxV, boxes, chunk, nchunk, minpres, &layout, &pctl);
								/* draw all chunks */
								drawall (&ev, 0, chunk, nchunk, boxes, minpres, &layout, y_offset, &pctl);
							}
							else {
								/* draw only chunks that are not yet drawn */
								drawall (&ev, 0, chunk, nchunk, boxes, minpres, &layout, y_offset, &pctl);
							}
						}
						else {
							/* update state fileIO */
							StateFileIO = cStateFileToBeClosed;
#ifdef _MAKE_POSTSCRIPT_
pctl.Do_PostScript = 1;
open_postscript(pctl);
drawall (&ev, 1, chunk, nchunk, boxes, minpres, &layout, y_offset, &pctl); 
XFlush (dpy);
exit(0);
#endif
						}
						break;

					case cStateFileToBeFreed:

						Dprintf (stderr, "==== File is to be freed ====\n");
						Dflush (stderr);
						/* update state */
						StateFileIO = cStateFileIsFreed;
						/* free chunks */
						for (Index = 0; Index < nchunk; Index = Index + 1) {
							free (chunk [Index].va);
							chunk [Index].va = NULL;
							sigDeleteSignal(chunk[Index].signal_stream);
						}
						nchunk = 0;
						break;

					case cStateFileIsFreed:

						Dprintf (stderr, "==== File is freed  ====\n");
						Dflush (stderr);
						/* nothing to do */
						break;
				}
			} /* endif XEventsQueued() */
		} /* endwhile !ChangeFile */
	} /* endwhile !QuitLoop */	
           
	pUnipen = upDelUnipen (pUnipen);
	return(0);
}



int scalesam (int force_scaling, int *pwidth, int *pheight, int *pmaxt, int *pmaxv,
	Boxtype boxes[],
	Chunktype chunk[MAXCHUNKS],
	int    nchunk,
	int    minpres,
	Layout *layout,
	Plotcontrol *pctl)
{
    int changed;

    if((pctl->scale_mode == SCALE_REAL_SETUP) && 
       (pctl->x_dim_given == 1) &&
       (pctl->y_dim_given == 1) 
       ) {
         changed = scalesam_real(force_scaling, pwidth, pheight, pmaxt, pmaxv, boxes, chunk, nchunk, minpres, layout, pctl);
    } else {
         changed = scalesam_auto(force_scaling, pwidth, pheight, pmaxt, pmaxv, boxes, chunk, nchunk, minpres, layout, pctl);
    }
    
    return (changed);
} /* End scalesam() */

int scalesam_real (
	int    force_scaling,
	int    *pwidth,
	int    *pheight,
	int    *pmaxt,
	int    *pmaxv,
	Boxtype boxes[],
	Chunktype chunk[MAXCHUNKS],
	int    nchunk,
	int    minpres,
	Layout *layout,
	Plotcontrol *pctl)
{
    int	ichunk;
    int scalew, scaleh; 
    float   scale = 1.0;
    int changed = 0;
	    
    if (force_scaling)
    {
      changed = 1;

      *pwidth  = 0;
      *pheight = 0;

      *pmaxt = 0;
      *pmaxv = 0;

      for (ichunk = 0; ichunk < nchunk; ichunk++)
      {
        chunk [ichunk].is_scaled = 0;
      }
    }


/* For all chunks, use the unipen_box for scaling */

    for (ichunk = 0; ichunk < nchunk; ichunk++) {
        if (!chunk [ichunk].is_scaled) {
	  if(chunk[ichunk].nxyz > *pmaxt) {
	      *pmaxt = chunk[ichunk].nxyz;
	  }
	}
    }
    for (ichunk = 0; ichunk < nchunk; ichunk++) {
        if (!chunk [ichunk].is_scaled) {
  	  chunk[ichunk].maxt = *pmaxt;

	  chunk[ichunk].minx = 0;
	  chunk[ichunk].maxx = pctl->x_dim;
	
	  chunk[ichunk].miny = 0; 
	  chunk[ichunk].maxy = pctl->y_dim;
	}	
    }

    for (ichunk = 0; ichunk < nchunk; ichunk++) {
        if (!chunk [ichunk].is_scaled) {
          chunk [ichunk].is_scaled = 1;

          scalew = pctl->x_dim;
          scaleh = pctl->y_dim;
	
          if(scalew < 1) scalew = 1;
          if(scaleh < 1) scaleh = 1;
	
          if( (float) boxes[ichunk+NCONTROLBOXES].h / scaleh < 
	      (float) boxes[ichunk+NCONTROLBOXES].w / scalew ) {
	       scale = (float) boxes[ichunk+NCONTROLBOXES].h / scaleh;
          } else {
	       scale = (float) boxes[ichunk+NCONTROLBOXES].w / scalew;
          }
          scale = 0.9 * scale;
        
	  chunk[ichunk].scale = scale;
	}
   } /* endfor ichunk */

   return (changed);

} /* End scalesam_real() */

int scalesam_auto (
	int    force_scaling,
	int    *pwidth,
	int    *pheight,
	int    *pmaxt,
	int    *pmaxv,
	Boxtype boxes[],
	Chunktype chunk[MAXCHUNKS],
	int    nchunk,
	int    minpres,
	Layout *layout,
	Plotcontrol *pctl)
{
    int	s, ichunk, isplit, nthick;
    int scalew, scaleh; 
    float   scale = 1.0;
	sigSignal *usignal;
    
    int changed = 0;
    
    nthick = pctl->nthick;
    if(nthick < 1) nthick = 1;


    if (force_scaling)
    {
      *pwidth  = 0;
      *pheight = 0;

      *pmaxt = 0;
      *pmaxv = 0;

      for (ichunk = 0; ichunk < nchunk; ichunk++)
      {
        chunk [ichunk].is_scaled = 0;
      }
    }


    if (pctl->veloc) {      /* scale velocity */
        for (ichunk = 0; ichunk < nchunk; ichunk++) {
            if (!chunk [ichunk].is_scaled) { 
                if(chunk[ichunk].nxyz > *pmaxt)
                    *pmaxt = chunk[ichunk].nxyz;
                chunk[ichunk].va[0] = 0;
        
                /* Remove velocities at penup */
        
                for (s = 1; s <  chunk[ichunk].nxyz; s++) {
                    if(chunk[ichunk].signal_stream->z[s] <= minpres) {
                        chunk[ichunk].va[s-1] = 0;
                        chunk[ichunk].va[s] = 0;
                        if (s+1 < chunk[ichunk].nxyz )
                            chunk[ichunk].va[s+1] = 0;
                    }
                    if(chunk[ichunk].va[s] > *pmaxv)
                        *pmaxv = chunk[ichunk].va[s];
                }
            }
        }
        if(*pmaxt < 1) *pmaxt = 1;
    	if(*pmaxv < 1) *pmaxv = 1;
        isplit = 2;
    } else {
        isplit = 1;	
    }
	
/* For all chunks, Find min/max for x/y of a rectangle bounding that chunk. */
	
    for (ichunk = 0; ichunk < nchunk; ichunk++) {
        if (!chunk [ichunk].is_scaled) { 
            chunk[ichunk].maxt = *pmaxt;
            chunk[ichunk].maxv = *pmaxv;
            
            chunk[ichunk].minx =  INFTY; chunk[ichunk].miny =  INFTY; 
            chunk[ichunk].maxx = -INFTY; chunk[ichunk].maxy = -INFTY;
        
            usignal = chunk[ichunk].signal_stream;
            for (s = 0; s <  chunk[ichunk].nxyz; s++) {
                if (usignal->z[s] > minpres) {
                    if (usignal->x[s] < chunk[ichunk].minx) chunk[ichunk].minx = usignal->x[s];
                        if (usignal->y[s] < chunk[ichunk].miny) chunk[ichunk].miny = usignal->y[s];
                        if (usignal->x[s] > chunk[ichunk].maxx) chunk[ichunk].maxx = usignal->x[s];
                        if (usignal->y[s] > chunk[ichunk].maxy) chunk[ichunk].maxy = usignal->y[s];
                }
            }
            if(pctl->scale_mode == SCALE_ON_BIGGEST) {
                if (chunk[ichunk].maxx - chunk[ichunk].minx > *pwidth ) {
                    changed = 1;
                    *pwidth = chunk[ichunk].maxx - chunk[ichunk].minx;
                }     
                if (chunk[ichunk].maxy - chunk[ichunk].miny > *pheight ) {
                    changed = 1;
                    *pheight = chunk[ichunk].maxy - chunk[ichunk].miny;
                }
            }
        }
    }
    
    for (ichunk = 0; ichunk < nchunk; ichunk++) {
        if (!chunk [ichunk].is_scaled) {
            chunk [ichunk].is_scaled = 1;
            if (pctl->scale_mode == SCALE_ON_BIGGEST) {
                scalew = *pwidth;
                scaleh = *pheight;
            } else {
                scalew = chunk[ichunk].maxx - chunk[ichunk].minx;
                scaleh = chunk[ichunk].maxy - chunk[ichunk].miny;
            }
            scalew += (nthick - 1);
            scaleh += (nthick - 1);
            if (scalew < 1) scalew = 1;
            if(scaleh < 1) scaleh = 1;
            if ( (float) boxes[ichunk+NCONTROLBOXES].h / scaleh 
               < (float) boxes[ichunk+NCONTROLBOXES].w / scalew ) {
                scale = (float) boxes[ichunk+NCONTROLBOXES].h / scaleh;
            } else {
                scale = (float) boxes[ichunk+NCONTROLBOXES].w / scalew;
            }
            scale = (1./(float) isplit) * 0.9 * scale;
            chunk[ichunk].scale = scale;
        }
    } /* endfor ichunk */

    return (changed);

} /* End scalesam_auto() */


void draw_unipen_box (
		  Chunktype *chunk,
		  Boxtype *b,
		  int y_offset,
		  Plotcontrol *pctl)
{
   int x1, y1, x2, y2, i;

   x1 = 0 + (0.05 * b->w) + b->x;
   y1 = 0 + (0.05 * b->h) + b->y - y_offset;
   x2 = x1 + (int) (0.5 + chunk->scale * (float) pctl->x_dim);
   y2 = y1 + (int) (0.5 + chunk->scale * (float) pctl->y_dim);

   XDrawRectangle(dpy, window1, gcSetupBox, x1, y1, x2-x1+1, y2-y1+1);

   for(i = 0; i < pctl->h_line_nlines; ++i) {  
   
      x1 = 0 + (0.05 * b->w) + b->x;
      y1 = 0 + (0.05 * b->h) + b->y - y_offset 
             + (int) (0.5 + chunk->scale 
                    * (float) (pctl->y_dim - pctl->h_lines[i]));
                    
      x2 = x1 + (int) (0.5 + chunk->scale * (float) pctl->x_dim);
      y2 = y1;

      XDrawLine(dpy, window1, gcSetupBox, x1, y1, x2, y2);
   }

} /* End draw_unipen_box() */

void draw_chunkbox (
		  Boxtype *b,
		  int y_offset,
		  Plotcontrol *pctl)
{
   double X1, Y1;	
   XRectangle within;
   Region reg = XCreateRegion();

   XClearArea(dpy, window1, b->x, b->y - y_offset, b->w, b->h,False);
   
   XDrawRectangle(dpy, window1, gcBox, b->x, b->y - y_offset, b->w, b->h);
   
   within.x = b->x + 1;
   within.y = b->y -y_offset + 1;
   within.width = b->w - 2;
   within.height = b->h - 2;

   XUnionRectWithRegion (&within,reg,reg);
   XSetRegion(dpy,gcName,reg);

#define dxBoxName 10
#define dyBoxName 20

	if (!pctl->no_labels) {
		XDrawImageString(dpy, window1, gcName, b->x + dxBoxName
			, b->y + dyBoxName - y_offset
			,b->descr, b->ldescr);
	}

   if(pctl->Do_PostScript && pctl->Do_PostScript_BoxTitle) {                       
   
       X1 = POST_SCALE *  (b->x + dxBoxName);
       Y1 = POST_SCALE * (740. - (b->y + dyBoxName - y_offset));
                                                  
       fprintf(pctl->F_PostScript,"%f %f moveto (%s) show\n",X1,Y1,make_stripped_label(b->descr));
   }                                       
                                      
	if(pctl->veloc) {                    
		XDrawImageString(dpy, window1, gcName
			, b->x + dxBoxName
			,b->y + dyBoxName + (b->h - 30)/2-y_offset
			,b->va_title, b->lva_title);
	}	                    
} /* End draw_chunkbox() */

void draw_controlbox (Boxtype *b)
{
	
   XPoint poly[5];
   int idx = 3, idy = 3;
   XRectangle within;
   Region reg = XCreateRegion();
	   
   XClearArea (dpy,window1,b->x,b->y,b->w+1,b->h+1,False);

   if(b->visible == 1) {
/* Left */
        poly[0].x = b->x;       poly[0].y = b->y;
        poly[1].x = b->x + idx; poly[1].y = b->y + idy;
        poly[2].x = b->x + idx; poly[2].y = b->y + b->h - idy;
        poly[3].x = b->x;       poly[3].y = b->y + b->h;
        poly[4].x = b->x;       poly[4].y = b->y;

        XDrawLines(dpy, window1, gcControlBox, poly, 5, CoordModeOrigin);
/* Right */
        poly[0].x = b->x + b->w + 1;       poly[0].y = b->y;
        poly[1].x = b->x + b->w - idx;     poly[1].y = b->y + idy;
        poly[2].x = b->x + b->w - idx;     poly[2].y = b->y + b->h - idy;
        poly[3].x = b->x + b->w + 1;       poly[3].y = b->y + b->h;
        poly[4].x = b->x + b->w + 1;       poly[4].y = b->y;
        XFillPolygon(dpy, window1, gcControlBox, poly, 5, Convex, CoordModeOrigin);
        
/* Top */
        poly[0].x = b->x;              poly[0].y = b->y;
        poly[1].x = b->x + idx;        poly[1].y = b->y + idy;
        poly[2].x = b->x + b->w - idx; poly[2].y = b->y + idy;
        poly[3].x = b->x + b->w;       poly[3].y = b->y;
        poly[4].x = b->x;              poly[4].y = b->y;
        XDrawLines(dpy, window1, gcControlBox, poly, 5, CoordModeOrigin);

/* Bottom */
        poly[0].x = b->x;              poly[0].y = b->y + b->h + 1;
        poly[1].x = b->x + idx;        poly[1].y = b->y + b->h - idy;
        poly[2].x = b->x + b->w - idx; poly[2].y = b->y + b->h - idy;
        poly[3].x = b->x + b->w;       poly[3].y = b->y + b->h + 1;
        poly[4].x = b->x;              poly[4].y = b->y + b->h + 1;
        XFillPolygon(dpy, window1, gcControlBox, poly, 5, Convex, CoordModeOrigin);

        within.x = b->x + idx + 1;
        within.y = b->y + idy + 1;
        within.width = b->w - 2 * idx - 2;
        within.height = b->h - 2 * idy - 2; 

        XUnionRectWithRegion (&within,reg,reg);
        XSetRegion(dpy,gcControlName,reg);

	XDrawImageString(dpy, window1, gcControlName, b->x + 6, b->y + idy + 12,
	   b->descr, b->ldescr);
          
   }
} /* End draw_controlbox() */

void flash_controlbox (Boxtype *b)
{
	
   XPoint poly[5];
   int idx = 3, idy = 3;
   XRectangle within;
   Region reg = XCreateRegion();
	   
   if(b->visible == 1) {
        XClearArea (dpy,window1,b->x,b->y,b->w,b->h,False);

/* Left */
        poly[0].x = b->x;       poly[0].y = b->y;
        poly[1].x = b->x + idx; poly[1].y = b->y + idy;
        poly[2].x = b->x + idx; poly[2].y = b->y + b->h - idy;
        poly[3].x = b->x;       poly[3].y = b->y + b->h;
        poly[4].x = b->x;       poly[4].y = b->y;

        XFillPolygon(dpy, window1, gcControlBox, poly, 5, Convex, CoordModeOrigin);
/* Right */
        poly[0].x = b->x + b->w;       poly[0].y = b->y;
        poly[1].x = b->x + b->w - idx; poly[1].y = b->y + idy;
        poly[2].x = b->x + b->w - idx; poly[2].y = b->y + b->h - idy;
        poly[3].x = b->x + b->w;       poly[3].y = b->y + b->h;
        poly[4].x = b->x + b->w;       poly[4].y = b->y;
        XDrawLines(dpy, window1, gcControlBox, poly, 5, CoordModeOrigin);
        
/* Top */
        poly[0].x = b->x;              poly[0].y = b->y;
        poly[1].x = b->x + idx;        poly[1].y = b->y + idy;
        poly[2].x = b->x + b->w - idx; poly[2].y = b->y + idy;
        poly[3].x = b->x + b->w;       poly[3].y = b->y;
        poly[4].x = b->x;              poly[4].y = b->y;
        XFillPolygon(dpy, window1, gcControlBox, poly, 5, Convex, CoordModeOrigin);

/* Bottom */
        poly[0].x = b->x;              poly[0].y = b->y + b->h;
        poly[1].x = b->x + idx;        poly[1].y = b->y + b->h - idy;
        poly[2].x = b->x + b->w - idx; poly[2].y = b->y + b->h - idy;
        poly[3].x = b->x + b->w;       poly[3].y = b->y + b->h;
        poly[4].x = b->x;              poly[4].y = b->y + b->h;
        XDrawLines(dpy, window1, gcControlBox, poly, 5, CoordModeOrigin);

        within.x = b->x + idx + 1;
        within.y = b->y + idy + 1;
        within.width = b->w - 2 * idx - 2;
        within.height = b->h - 2 * idy - 2; 

        XUnionRectWithRegion (&within,reg,reg);
        XSetRegion(dpy,gcControlName,reg);

	XDrawImageString(dpy, window1, gcControlName, b->x + 6, b->y + idy + 12,
	   b->descr, b->ldescr);
          

   }

} /* End flash_controlbox() */

void flash_chunkbox (Boxtype *b, int y_offset)
{
	int i;

	for (i = 1; i < 4; i++) {
		XDrawRectangle(dpy, window1, gcFlashBox, b->x+i
		         , b->y+ i  
                         , b->w-i*2, b->h-i*2);
	}
	XFlush(dpy);
} /* End flash_chunkbox() */

void drawall (
		  XEvent *ev,
		  int force_draw,
		  Chunktype chunk[MAXCHUNKS],
		  int nchunk,
		  Boxtype boxes[],
		  int minpres,
		  Layout *layout,
		  int y_offset,
		  Plotcontrol *pctl)
{
	int i, ichunk, ibox;
	double x1, y1, z1, x2, y2, z2, t1, t2, va1, va2;
	int first_visible;
	int ithick;
	int npendown;

	va1 = va2 = 0;

	/* Begin: Overall appearance -----------------------------------------------------*/	
	if (force_draw != 0) {
		if(force_draw == 1) {
			XClearArea(dpy, window1, 0 , 0, WINDW-2*BUTTONCOLW+1, WINDH, False);
			XClearArea (dpy,window1,0,0,window_paint,Y_TITLE_AREA,False);
		} else if(force_draw == 2){ 
			XClearWindow(dpy, window1);
			mark_boxes(boxes, cBoxStateChanged);
		}
		/* Always draw file name */
		XDrawImageString(dpy, window1, gcFileName, BUTD, Y_TITLE_AREA - BUTD 
			,chunk[0].file_name, chunk[0].lfile_name);
		/* request redrawing of all chunks */
		for (i = 0; i < MAXCHUNKS; i++) {
			chunk [i].is_drawn = 0;
		}
	}

	/* Draw the control buttons */
	for (i = 0; i < NCONTROLBOXES; i++) {
		if(boxes[i].state != cBoxStateUnchanged) {
			draw_controlbox(&boxes[i]);
			boxes[i].state = cBoxStateUnchanged;
		}
	}

	/* Begin: individual box content ------------------------------------------------*/

	t1 = t2 = 0;
	/* Draw the curves for each chunk */
	first_visible = -1;
	for (ichunk = 0; ichunk < nchunk; ++ichunk) {
		ibox = NCONTROLBOXES + ichunk;
		if (!chunk [ichunk].is_drawn) {
			if( VISIBLE_BOX(ibox)) {
				if(first_visible == -1) {
					first_visible = 1;
					layout->upper_left_chunk = ichunk;
				}
				layout->lower_right_chunk = ichunk;
				chunk [ichunk].is_drawn = 1;
				/* draw bounding box around each chunk */
				draw_chunkbox(&boxes[ibox], y_offset, pctl);
				if(pctl->scale_mode == SCALE_REAL_SETUP) {
					draw_unipen_box(&chunk[ichunk],&boxes[ibox],y_offset,pctl);
				}
				scale_sample(boxes, chunk, ichunk, 0, pctl, &x1, &y1, &va1);
				npendown = 0;
				y1 -= y_offset;
				z1 = chunk[ichunk].signal_stream->z[0];
				va1 -= y_offset;	    
				if (pctl->veloc) {
					t1 = BUTD + boxes[ibox].x + (0 * (boxes[ibox].w - 2*BUTD)) / chunk[ichunk].maxt;
				}
				if (chunk[ichunk].nxyz==1) {
					if (z1<=minpres) {
						draw_dot(gcAir,x1,y1,pctl->nthick);
					} else {
						draw_dot(gcInk,x1,y1,pctl->nthick);
						npendown = 1;
					}
				}
				for(i = 1; i < chunk[ichunk].nxyz; ++i) {
					scale_sample(boxes, chunk, ichunk, i, pctl, &x2, &y2, &va2);
					y2 -= y_offset;
					z2 = chunk[ichunk].signal_stream->z[i];
					va2 -= y_offset;
					if(z1 <= minpres || z2 <= minpres) {
						if( ! pctl->Do_PostScript ) {/* Do not show penup in PostScript */
							if(z1 != cPenAdded && z2 != cPenAdded) {
								if(pctl->show_penup) {
									draw_dot(gcAir, x1, y1, pctl->nthick);
								}
							}
						}
					} else {
						if( !pctl->gesture_mode ) {
							draw_line(gcInk, x1, y1, x2, y2, pctl, pctl->nthick);
						} else {
							ithick = (pctl->nthick * (chunk[ichunk].nxyz - i)) / chunk[ichunk].nxyz;
							if(ithick < 1) ithick = 1;
								draw_line(gcInk, x1, y1, x2, y2, pctl, ithick);
						}
						npendown++;
					}
					if(pctl->veloc) {
						t2 = BUTD + boxes[ibox].x + (i * (boxes[ibox].w - 2*BUTD)) / chunk[ichunk].maxt;
						if( (z1 > minpres) && (z2 > minpres) ) {
							draw_line(gcInk, t1, va1, t2, va2, pctl, 1);
						}
					}

					x1 = x2;
					y1 = y2;
					z1 = z2;
					va1 = va2;
					t1 = t2;
				} /* endfor i sample */
			} /* endif VISIBLE 	 */
		} /* endif chunk not drawn */
	} /* endfor ichunk */
	if(pctl->Do_PostScript) {
		close_postscript(pctl);
	}
	pctl->Do_PostScript = 0;
} /* End drawall() */
/* handle_buttonpress now returns 1 if a redraw is needed */

int handle_buttonpress (
		  tUPUnipen *_pUnipen_, /* unipen */
		  XEvent *ev,
		  Boxtype boxes[],
		  Chunktype chunk[],
		  int nchunk,
		  int *quit_request,
		  int *y_offset,
		  Plotcontrol *pctl,
		  int *ncols_requested,
		  int *level_requested,
		  int level_max,
		  Layout *layout,
		  char current_unipen_file[],
		  int StateFileIO)
{
   int b, last_box, irow, jchunk;
	
   last_box = NCONTROLBOXES + nchunk - 1;
	
   for (b = 0; (!INBOX(boxes[b], ev->xbutton.x, ev->xbutton.y)) &&
		(b < NCONTROLBOXES + nchunk); b++) ;
		
   if (b > last_box) {
	return 0;
   }

   if(boxes[b].visible == 1) {
   	    boxes[b].state = cBoxStateChanged;
	
		if (b < NCONTROLBOXES) {
	        flash_controlbox(&boxes[b]);
   	        
		if (strcmp(boxes[b].descr, LAB_QUIT) == 0) {
			*quit_request = USR_QUIT;
			finish_file(boxes);
			return 1;
			
		} else if (strcmp(boxes[b].descr, LAB_NEXTF) == 0) {
			*y_offset = 0;
			*quit_request = USR_NEXTF;
			finish_file(boxes);
			return 1;
			
		} else if (strcmp(boxes[b].descr, LAB_PREVF) == 0) {
			*y_offset = 0;
			*quit_request = USR_PREVF;
			finish_file(boxes);
			return 1;
			
		} else if (strcmp(boxes[b].descr, LAB_DOWN) == 0) {
			if(nchunk > 0) {
			   *y_offset = *y_offset + (layout->nrows/2) * boxes[last_box].h;
			   bound_y_offset(y_offset, boxes, last_box);
				return 1;
			} else
				return 0;
		
		} else if (strcmp(boxes[b].descr, LAB_UP) == 0) {
			if(nchunk > 0) {
			   *y_offset = *y_offset - (layout->nrows/2) * boxes[last_box].h;
			   bound_y_offset(y_offset, boxes, last_box);
				return 1;
			} else
				return 0;

		} else if (strcmp(boxes[b].descr, LAB_LESS_INK) == 0) {
			--(pctl->nthick);
			if(pctl->nthick < 1) {
			   pctl->nthick = 1; 
			   XBell(dpy,25); 
				return 0;
			} else
				return 1;
		
		} else if (strcmp(boxes[b].descr, LAB_MORE_INK) == 0) {
			++(pctl->nthick);
			if(pctl->nthick > MAX_INK_THICKNESS) {
				pctl->nthick = MAX_INK_THICKNESS;
				XBell(dpy,25); 
				return 0;
			} else
				return 1;

		} else if (strcmp(boxes[b].descr, LAB_ZOOM_IN) == 0) {
			--(*ncols_requested);
			if(*ncols_requested < 1) {
			   *ncols_requested = 1; 
			   XBell(dpy,25); 
				return 0;
			}
			irow = *y_offset / (layout->lboxheight);
			*y_offset = (irow * (*ncols_requested + 1)) 
				* (layout->lboxheight) / *ncols_requested;
			*quit_request = USR_ZOOM;
			return 1;
		
		} else if (strcmp(boxes[b].descr, LAB_ZOOM_OUT) == 0) {
			++(*ncols_requested);
			if(*ncols_requested > MAX_COLS_REQUESTED) {
				*ncols_requested = MAX_COLS_REQUESTED;
				XBell(dpy,25); 
				return 0;
			}
			irow = *y_offset / (layout->lboxheight);
			*y_offset = (irow * (*ncols_requested - 1)) 
				* (layout->lboxheight) / *ncols_requested;
			*quit_request = USR_ZOOM;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_HIER_UP) == 0) {
			--(*level_requested);
			if(*level_requested < 0) {
			   *level_requested = 0; 
			   XBell(dpy,25); 
				return 0;
			}
			*quit_request = USR_HIERARCHY;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_HIER_DOWN) == 0) {
			++(*level_requested);
			if(*level_requested > level_max) {
				*level_requested = level_max;
				XBell(dpy,25); 
				return 0;
			}
			*quit_request = USR_HIERARCHY;
			return 1;
                        
		} else if (strcmp(boxes[b].descr, LAB_PRINT) == 0) {
			XBell(dpy,25); 
			pctl->Do_PostScript = 1;
			open_postscript(pctl);
			*quit_request = USR_ZOOM;
			return 0;
/*
                        
		} else if (strcmp(boxes[b].descr, LAB_SCALE_PER_CHUNK) == 0) {
		        pctl->scale_mode = SCALE_ON_BIGGEST;
			*quit_request = USR_ZOOM;

		} else if (strcmp(boxes[b].descr, LAB_SCALE_ON_BIGGEST) == 0) {
                        if ( (upGetArgument(_pUnipen_, cKeyXDim, 0) == NULL) 
                          || (upGetArgument(_pUnipen_, cKeyYDim, 0) == NULL) ) {
		              pctl->scale_mode = SCALE_PER_CHUNK;
			} else {
		              pctl->scale_mode = SCALE_REAL_SETUP;
			}
			*quit_request = USR_ZOOM;

		} else if (strcmp(boxes[b].descr, LAB_SCALE_REAL_SETUP) == 0) {
		        pctl->scale_mode = SCALE_PER_CHUNK;
			*quit_request = USR_ZOOM;
*/
      } else if (was_scale_request(boxes[b].descr,pctl)) {
			*quit_request = USR_ZOOM;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_VELOC) == 0) {
			pctl->veloc = 0;
			*quit_request = USR_ZOOM;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_NOVELOC) == 0) {
			pctl->veloc = 1;
			*quit_request = USR_ZOOM;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_GESTURE) == 0) {
			pctl->gesture_mode = 0;	
			pctl->nthick = 1;
			*quit_request = USR_ZOOM;
			return 1;
		
		} else if (strcmp(boxes[b].descr, LAB_NOGEST) == 0) {
			pctl->gesture_mode = 1;	
			pctl->nthick = INITIAL_GESTURE_THICKNESS;
			*quit_request = USR_ZOOM;
			return 1;
                        
		} else if (strcmp(boxes[b].descr, LAB_OPENED_LINES) == 0) {
			pctl->open_lines = 0;	
			*quit_request = USR_ZOOM;
			return 1;
		
		} else if (strcmp(boxes[b].descr, LAB_FILLED_LINES) == 0) {
			pctl->open_lines = 1;	
			if(pctl->nthick <= 1) {
				pctl->nthick = INITIAL_GESTURE_THICKNESS;
			}
			*quit_request = USR_ZOOM;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_ROUNDED_LINES) == 0) {
			pctl->rounded_lines = 0;	
			*quit_request = USR_ZOOM;
			return 1;
		
		} else if (strcmp(boxes[b].descr, LAB_JAGGED_LINES) == 0) {
			pctl->rounded_lines = 1;	
			if(pctl->nthick <= 1) {
				pctl->nthick = INITIAL_GESTURE_THICKNESS;
			}
			*quit_request = USR_ZOOM;                   		
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_SHOW_PENUP) == 0) {
			pctl->show_penup = 0;	
			*quit_request = USR_ZOOM;
			return 1;

		} else if (strcmp(boxes[b].descr, LAB_NOSHOW_PENUP) == 0) {
			pctl->show_penup = 1;	
			*quit_request = USR_ZOOM;
			return 1;

		}
	} else {
		/* then b is a chunk box */
		if(StateFileIO == cStateFileIsClosed) {
			jchunk = b + layout->upper_left_chunk - NCONTROLBOXES;
			fprintf (stderr,"CLICKED %d %d %d %d\n"
				,jchunk,b,layout->upper_left_chunk,layout->lower_right_chunk);
			if(jchunk <= layout->lower_right_chunk) {
				flash_chunkbox(&boxes[b], *y_offset);
				boxes[b].state = cBoxStateChanged;
				handle_word_clicked(chunk,jchunk,current_unipen_file);
			}
		} else {
			fprintf(stdout,"%c(please wait...)", BELL); fflush(stdout);
		}
		return 0;
	}

    } /* boxes[].visible */
	 return 0;
} /* End handle_buttonpress() */

void set_layout (int ncols_requested, Layout *layout)
{  
   layout->windw = WINDW;
   layout->windh = WINDH;
   layout->lboxwidth = (layout->windw - 2 * BUTTONCOLW) / ncols_requested;
   layout->lboxheight = layout->lboxwidth / ASPECTW;
   layout->maxwholechunk = layout->windw / 2;
   layout->ncolumns = (layout->windw - 2 * BUTTONCOLW) / layout->lboxwidth;
   layout->nrows = layout->windh / layout->lboxheight;
   layout->upper_left_chunk = 0;
   layout->lower_right_chunk = 0;
} /* End set_layout */

int chunkboxx (int ichunk, Layout *layout)
{
   return(((ichunk) % (layout->ncolumns)) * layout->lboxwidth);
} /* End chunkboxx() */

int chunkboxy (int ichunk, Layout *layout)
{
   return(((ichunk) / (layout->ncolumns)) * layout->lboxheight);
} /* End chunkboxy() */

void setup_a_box (Boxtype *box, int x, int y, int w, int h
                    , char *lab, char *va_title, int state, int visible) 
{
	box->x = x;
	box->y = y;
	box->w = w;
	box->h = h;
	strcpy(box->descr, lab);
	box->ldescr = strlen(lab);
	strcpy(box->va_title, va_title);
	box->lva_title = strlen(va_title);
	box->state = state;
	box->visible = visible;
}


void setup_boxes (
		  Boxtype boxes[],
		  Chunktype chunk[MAXCHUNKS],
		  int nchunk,
		  Layout *layout,
		  int level_max,
		  Plotcontrol *pctl)
{
   register int i, iy;
   static int init = 0; 
  
   static Boxtype	ctlboxes[NCONTROLBOXES];

   if(init == 0) {
       iy = 0 + BUTD + .5 * BUTTONROWH;
       
       setup_a_box(&ctlboxes[BUT_PREVF]
                               , WINDW - 2 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_PREVF," ", 0,1);

       setup_a_box(&ctlboxes[BUT_NEXTF]
                               , WINDW - 1 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_NEXTF," ", 0,1);

       iy += BUTTONROWH;
       
       setup_a_box(&ctlboxes[BUT_MORE_INK]
                               , WINDW - 2 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_MORE_INK, " ", 0,1);

       setup_a_box(&ctlboxes[BUT_LESS_INK]
                               , WINDW - 1 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_LESS_INK, " ", 0,1);

       iy += BUTTONROWH;
       
       setup_a_box(&ctlboxes[BUT_ZOOM_IN]
                               , WINDW - 2 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_ZOOM_IN, " ",  0,1);

       setup_a_box(&ctlboxes[BUT_ZOOM_OUT], WINDW - 1 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_ZOOM_OUT, " ", 0,1);

       iy += BUTTONROWH;
       
       setup_a_box(&ctlboxes[BUT_DOWN]
                               , WINDW - 1 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_DOWN, " ", 0,1);

       setup_a_box(&ctlboxes[BUT_UP], WINDW - 2 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_UP," ", 0,1);

       iy += BUTTONROWH;
       
       setup_a_box(&ctlboxes[BUT_HIER_UP]
                               , WINDW - 2 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_HIER_DOWN, " ",  0,1);

       setup_a_box(&ctlboxes[BUT_HIER_DOWN]
                                , WINDW - 1 * BUTTONCOLW + BUTD
                                , iy
                                , BUTW, BUTH, LAB_HIER_UP, " ", 0,1);

       iy += BUTTONROWH;

#define WIDE_BUTH ( (int) ((double) BUTH * 0.66 +0.5))
#define WIDE_BUTW (BUTW*2)

       setup_a_box(&ctlboxes[BUT_SCALE]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , WIDE_BUTW, WIDE_BUTH, LAB_SCALE_PER_CHUNK," ", 0,1);
              
       iy += (WIDE_BUTH + BUTD);

       setup_a_box(&ctlboxes[BUT_VELOC]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , WIDE_BUTW, WIDE_BUTH, LAB_NOVELOC, " ", 0,1);

       iy += (WIDE_BUTH + BUTD);

       setup_a_box(&ctlboxes[BUT_GESTURE]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , WIDE_BUTW, WIDE_BUTH, LAB_NOGEST, " ", 0,1);

       iy += (WIDE_BUTH + BUTD);

       setup_a_box(&ctlboxes[BUT_OPEN]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , WIDE_BUTW, WIDE_BUTH, LAB_OPENED_LINES," ", 0,1);

       iy += (WIDE_BUTH + BUTD);

       setup_a_box(&ctlboxes[BUT_ROUNDED]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , WIDE_BUTW, WIDE_BUTH, LAB_ROUNDED_LINES," ", 0,1);

       iy += (WIDE_BUTH + BUTD);

       setup_a_box(&ctlboxes[BUT_PENUP]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , WIDE_BUTW, WIDE_BUTH, LAB_SHOW_PENUP," ", 0,1);
	
       iy = WINDH - BUTTONROWH;

       setup_a_box(&ctlboxes[BUT_PRINT]
                                , WINDW - 2 * BUTTONCOLW + BUTD
                                , iy
                                , BUTW, BUTH, LAB_PRINT, " ", 0,1);

       setup_a_box(&ctlboxes[BUT_QUIT]
                               , WINDW - 1 * BUTTONCOLW + BUTD
                               , iy
                               , BUTW, BUTH, LAB_QUIT, " ", 0,1);


       for (i = 0; i < NCONTROLBOXES; i++) {
	  boxes[i] = ctlboxes[i];
       } 
       init = 1;
   }
   
   for (i = 0; i < nchunk; i++) {
      boxes[i + NCONTROLBOXES].x = chunkboxx(i,layout);
      boxes[i + NCONTROLBOXES].y = chunkboxy(i,layout) + Y_TITLE_AREA;
      boxes[i + NCONTROLBOXES].w = layout->lboxwidth;
      boxes[i + NCONTROLBOXES].h = layout->lboxheight;
      sprintf(boxes[i + NCONTROLBOXES].descr, "%d %s",i,chunk[i].correct_name);
      sprintf(boxes[i + NCONTROLBOXES].va_title, "|D(k)| %d samples"
         ,chunk[i].nxyz);
      boxes[i + NCONTROLBOXES].visible = 1;	                                                 
   }

   if(pctl->scale_mode == SCALE_ON_BIGGEST) {
        strcpy(boxes[BUT_SCALE].descr, LAB_SCALE_ON_BIGGEST);
   } else if (pctl->scale_mode == SCALE_PER_CHUNK) {
        strcpy(boxes[BUT_SCALE].descr, LAB_SCALE_PER_CHUNK);
   } else if (pctl->scale_mode == SCALE_REAL_SETUP) {
        strcpy(boxes[BUT_SCALE].descr, LAB_SCALE_REAL_SETUP);
   }
   
   if(pctl->gesture_mode == 1) {
        strcpy(boxes[BUT_GESTURE].descr, LAB_GESTURE);
   } else {
        strcpy(boxes[BUT_GESTURE].descr, LAB_NOGEST);
   }
  
   if(pctl->veloc == 1) {
        strcpy(boxes[BUT_VELOC].descr, LAB_VELOC);
   } else {
        strcpy(boxes[BUT_VELOC].descr, LAB_NOVELOC);
   }

   if(pctl->open_lines == 1) {
        strcpy(boxes[BUT_OPEN].descr, LAB_OPENED_LINES);
   } else {
        strcpy(boxes[BUT_OPEN].descr, LAB_FILLED_LINES);
   }

   if(pctl->rounded_lines == 1) {
        strcpy(boxes[BUT_ROUNDED].descr, LAB_ROUNDED_LINES);
   } else {
        strcpy(boxes[BUT_ROUNDED].descr, LAB_JAGGED_LINES);
   }
  
   if(pctl->show_penup == 1) {
        strcpy(boxes[BUT_PENUP].descr, LAB_SHOW_PENUP);
   } else {
        strcpy(boxes[BUT_PENUP].descr, LAB_NOSHOW_PENUP);
   }
  
 
   for (i = 0; i < NCONTROLBOXES + nchunk; i++) {
      boxes[i].ldescr    = strlen(boxes[i].descr);
      boxes[i].lva_title = strlen(boxes[i].va_title);
   }
   
   if(level_max == 0) {
       /* No point in displaying hierarchy buttons */
        boxes[BUT_HIER_DOWN].visible = 0;
        boxes[BUT_HIER_UP].visible = 0;
   }
   else {
        boxes[BUT_HIER_DOWN].visible = 1;
        boxes[BUT_HIER_UP].visible = 1;
   }
   
} /* End setup_boxes() */

void parse_args (
			int argc,
			char *argv[],
			int *minpres,
			int *ncols_requested,
			int *scale_mode,
			int *y_flip,
			int *veloc,
			int *nthick,
			int *level_requested,
			int *gesture_mode,
			int *show_penup,
			int *first_arg,
			int *do_poll,
			int *no_labels,
			char unipen_definition_file[])
{
	register int i;

	*minpres = 0;
	*ncols_requested = 3;
	*scale_mode = SCALE_ON_BIGGEST;
	*y_flip = 0;
	*veloc = 0;
	*nthick = 1;
	*gesture_mode = 0;
	*show_penup = 0;
	*level_requested = 0;
	*do_poll = 0;
	*no_labels = 0;
	unipen_definition_file[0] = '\0';

   i = 1; 

   while((i < argc) && (argv[i][0] == '-') ) {
        if(argv[i][1] == 'p' ) {
            ++i;
				*minpres = atoi(argv[i]);
				printf("Option -p found\n");
 
        } else if(argv[i][1] == 'c' ) {
				++i;
            *ncols_requested = atoi(argv[i]);
            if(*ncols_requested < 1) *ncols_requested = 1;
            if(*ncols_requested > MAX_COLS_REQUESTED) *ncols_requested = MAX_COLS_REQUESTED;
            printf("Option -c found\n");

        } else if(argv[i][1] == 'i' ) {
            ++i;
					*nthick = atoi(argv[i]);
            if(*nthick < 1) *nthick = 1;
            if(*nthick > MAX_INK_THICKNESS) *nthick = MAX_INK_THICKNESS;
            printf("Option -i found\n");

        } else if(strcmp(argv[i],"-NL")==0) {
					printf ("Option -NL found, not displaying labels.....\n");
            *no_labels = 1;

        } else if(argv[i][1] == 'g' ) {
            *gesture_mode = 1;

        } else if(argv[i][1] == 'h' ) {
            ++i;
            *level_requested = atoi(argv[i]);
            if(*level_requested < 0) *level_requested = 0;
            if(*level_requested > MAX_LEVEL_REQUESTED) {
                   *level_requested = MAX_LEVEL_REQUESTED;
            }
            printf("Option -l found\n");

        } else if(argv[i][1] == 'f' ) {
            ++i;
            strcpy(unipen_definition_file,argv[i]);
            printf("Option -f [%s] found\n",unipen_definition_file);

        } else if(argv[i][1] == 'B' ) {
            *scale_mode = SCALE_ON_BIGGEST;
            printf("Option -B found\n");

        } else if(argv[i][1] == 'O' ) {
            *scale_mode = SCALE_PER_CHUNK;
            printf("Option -O found\n");

        } else if(argv[i][1] == 'S' ) {
            *scale_mode = SCALE_REAL_SETUP;
            printf("Option -S found\n");

        } else if(argv[i][1] == 'Y' ) {
            *y_flip = 1;
            printf("Option -Y found\n");

        } else if(argv[i][1] == 'V' ) {
            *veloc = 1;
            printf("Option -V found\n");

        } else if(argv[i][1] == 'W' ) {
            window_width = atoi(argv[i+1]);
            if(window_width < 300 ) {
            	window_width = 300;
            	printf("-W truncated to %d\n", window_width);
            }
            window_paint = window_width - 2 * BUTTONCOLW - BUTD;
	    
            i++;
            printf("Option -W found, setting width to %d\n",window_width);

        } else if(argv[i][1] == 'H' ) {
            window_height = atoi(argv[i+1]);
            if(window_height < 300 ) {
            	window_height = 300;
            	printf("-H truncated to %d\n", window_height);
            }
            i++;
            printf("Option -H found, setting height to %d\n",window_height);

        } else if(argv[i][1] == 'P' ) {
            *do_poll = 1;
            printf("Option -P found, upview polling for file\n");

        } else if(argv[i][1] == 'U' ) {
            *show_penup = 1;
            printf("Option -U found, also showing .PENUP signal\n");

        } else if(argv[i][1] == 'q' ) {
            sscanf (argv[i+1],"%f",&minQ);
				i++;
            printf("Option -q found, only showing segments with Q>%f\n",minQ);

        } else if(argv[i][1] == 'Q' ) {
            sscanf (argv[i+1],"%f",&maxQ);
				i++;
            printf("Option -Q found, only showing segments with Q<%f\n",maxQ);

        } else {
            print_usage();
            exit(1);
        }
        ++i;
   }
   printf("Option -p: taking minimum PenDown pressure %d\n", *minpres);
   printf("Option -c: taking #columns %d\n", *ncols_requested);
   printf("Option -i: taking line thickness in X11 pixels: %d\n", *nthick);
   printf("Option -h: taking hierarchy: %d\n", *level_requested);
   if(*scale_mode == SCALE_ON_BIGGEST) {
        printf("Option -B: biggest chunk size determines all scaling\n");
   } else if(*scale_mode == SCALE_PER_CHUNK) {
        printf("Option -O: optimal scaling per chunk (periods and comma's are inflated)\n");
   } else if(*scale_mode == SCALE_REAL_SETUP) {
        printf("Option -S: scaling determined by .X_DIM and .Y_DIM\n");
   }
   if(*y_flip) {
        printf("Option -Y: multiply Y channel by -1\n");
   } else {
        printf("Option -Y absent: do not multiply Y channel by -1\n");
   }
   if(*gesture_mode) {
        printf("Option -g: draw gestures with thick start, thin tail\n");
   } else {
        printf("Option -g absent: do not draw with thick start, thin tail\n");
   }
   if(*veloc) {
        printf("Option -V: (pseudo)velocity displayed for digitizer assessment\n");
        if(*scale_mode == SCALE_REAL_SETUP) {
                fprintf(stderr,"Do not give -V (velocity) and -S (.X_DIM .Y_DIM scaling) together\n");
                exit(1);
        }
   } else {
        printf("Option -V absent: do not display (pseudo)velocity\n");
   }

   if(unipen_definition_file[0] == '\0') {
      if (getenv(cUnipenDefinition)==NULL) {
         strcpy(unipen_definition_file,"unipen.def");
         printf("Option -f? taking %s us UNIPEN definition file\n"
            ,unipen_definition_file);

      } else {
         strcpy(unipen_definition_file,getenv(cUnipenDefinition));
         printf("Option -f? taking %s from environment variable %s\n"
            ,unipen_definition_file
            ,cUnipenDefinition);
      }
   }

	if (i==argc) {
		fprintf (stderr,"no files given as argument!\n");
		print_usage();
		exit(1);
	}
   *first_arg = i;
   fprintf(stdout,"First file argument is %s\n", argv[i]);

} /* End parse_args() */


void print_usage (void)
{
   fprintf(stderr, "Usage: \n   upview [-options] <wildspec of UniPen files>\n\n");

   printf("Options:\n");
   fprintf(stdout,"    -p <number>:  PenDown pressure criterion, e.g., -p0 inks all p>0\n");
   fprintf(stdout,"    -c <number>:  number of columns in display, e.g., -c3\n");
   fprintf(stdout,"    -i <number>:  line thickness of Ink in X11 pixels, e.g., -i4\n");
   fprintf(stdout,"    -h <number>:  requested hierarchy level (differs per file), e.g., -h0 is top level\n\n");
   fprintf(stdout,"    -B:           biggest chunk size determines all scaling\n");
   fprintf(stdout,"    -S:           setup .X_DIM/.Y_DIM determines all scaling\n");
   fprintf(stdout,"Or: -O:           optimal scaling per chunk (periods and comma's are inflated)\n\n");
   fprintf(stdout,"    -Y:           multiply Y channel by -1\n");
   fprintf(stdout,"    -U:           also show .PENUP signal\n");
   fprintf(stdout,"    -V:           display (pseudo)velocity for digitizer assessment\n");
   fprintf(stderr,"    -f <file>:    the UNIPEN definition file (default=unipen.def)\n");
   fprintf(stderr,"    -W <width>:  (default=%d)\n",window_width);
   fprintf(stderr,"    -H <height>: (default=%d)\n",window_height);
   fprintf(stderr,"    -NL:          do not display labels\n");
} /* End print_usage() */

void draw_line (
		  GC gc,
		  double x1,
		  double y1,
		  double x2,
		  double y2,
		  Plotcontrol *pctl,
		  int mthick)
{
#define THICK 12. /* Pixels */
    register int nthick, idx, idy, iw, ih;
    double ddx, ddy, cosine, sine, r;
    XPoint poly[7];
    double X1, Y1, X2, Y2;
    
    nthick = mthick;
    if(nthick < 1) nthick = 1;

   if(nthick > 1 && !pctl->Do_PostScript) {
      ddy = y2 - y1;
      ddx = x2 - x1;
      r = sqrt(ddx*ddx + ddy*ddy);
      cosine = 0.;
      sine = 0.;
      if(r != 0.0) {
         cosine = ddx/r;
         sine = ddy/r;
      }
      idx = nint4(((double) nthick)/2. * cosine);
      idy = nint4(((double) nthick)/2. * sine);
 
      poly[0].x = x1 - idx; poly[0].y = y1-idy;
      poly[1].x = x1 - idy; poly[1].y = y1+idx;
      poly[2].x = x2 - idy; poly[2].y = y2+idx;
      poly[3].x = x2 + idx; poly[3].y = y2+idy;
      poly[4].x = x2 + idy; poly[4].y = y2-idx;
      poly[5].x = x1 + idy; poly[5].y = y1-idx;
      poly[6].x = x1 - idx; poly[6].y = y1-idy;
    
      if(pctl->open_lines) {
         XDrawLines(dpy, window1, gc, poly, 7, CoordModeOrigin); 
      } else {
         XFillPolygon(dpy, window1, gc, poly, 7, Convex, CoordModeOrigin);
      }

      if(pctl->rounded_lines) {
         iw = ih = nthick;
         X1 = x1;
         Y1 = y1;
         XFillArc(dpy, window1, gc
            , nint4(X1-(double) iw/2.)
            , nint4(Y1-(double) ih/2.), iw, ih, 0, 64*360);
         X1 = x2;
         Y1 = y2;
         XFillArc(dpy, window1, gc
            , nint4(X1-(double) iw/2.)
            , nint4(Y1-(double) ih/2.), iw, ih, 0, 64*360);                
      }
 
   } else {
      XDrawLine(dpy, window1, gc, nint4(x1), nint4(y1), nint4(x2), nint4(y2));
        
      if(pctl->Do_PostScript) {
         X1 = POST_SCALE *  x1;
         Y1 = POST_SCALE * (740. - y1);
         X2 = POST_SCALE *  x2;
         Y2 = POST_SCALE * (740. - y2);
            
         if(nthick != 1 || pctl->gesture_mode ) {
            fprintf(pctl->F_PostScript,"%f setlinewidth\n"
               , POST_THICK * (double) nthick);
         }
         fprintf(pctl->F_PostScript,"n %.2f %.2f m %.2f %.2f l s\n", X1, Y1, X2, Y2);
      }
   }
} /* End draw_line() */

void draw_dot (
		  GC gc,
		  double x1,
		  double y1,
		  int thick)
{
#define THICK 12. /* Pixels */
    register int nthick, iw, ih;

    double x2;
    double y2;
  
    nthick = thick;
    if(nthick < 1) nthick = 1;

    if(nthick > 1) {
        iw = ih = nthick;
        XFillArc(dpy, window1, gc
                , nint4(x1-(double) iw/2.)
                , nint4(y1-(double) ih/2.), iw, ih, 0, 64*360);

 
    } else {
        x2 = x1;
        y2 = y1;
        XDrawLine(dpy, window1, gc, nint4(x1), nint4(y1), nint4(x2), nint4(y2));

    }
} /* End draw_dot() */

int nint4 (double r)
{
	int iret;

	if(r >= 0.0) {
		iret = (int) (r + 0.5);
	} else {
		iret = (int) (r - 0.5);
	}
	return(iret);
} 


void scale_sample_real (
		  Boxtype boxes[],
		  Chunktype chunk[],
		  int ichunk,
		  int s,
		  int y_flip,
		  double *xx,
		  double *yy)
{
   double x, y;

   x = chunk[ichunk].signal_stream->x[s];
   y = chunk[ichunk].signal_stream->y[s];

   if(y_flip ) {
	    y = y - chunk[ichunk].maxy;
   } else {
	    y = chunk[ichunk].maxy - y;
   }


   x = (0.05 * boxes[ichunk+NCONTROLBOXES].w) + chunk[ichunk].scale * x;
   y = (0.05 * boxes[ichunk+NCONTROLBOXES].h) + chunk[ichunk].scale * y;

   x += boxes[ichunk + NCONTROLBOXES].x;
   y += boxes[ichunk + NCONTROLBOXES].y;
   

   *xx = x;
   *yy = y;
}

void scale_sample_auto (
		  Boxtype boxes[],
		  Chunktype chunk[],
		  int ichunk,
		  int s,
		  Plotcontrol *pctl,
		  double *xx,
		  double *yy,
		  double *vv)
{
	register int isplit;
	double x, y, va;
	
	if(pctl->veloc) {
		isplit = 2;
	} else {
		isplit = 1;
	}
	
	x = chunk[ichunk].signal_stream->x[s];
	y = chunk[ichunk].signal_stream->y[s];
	va = chunk[ichunk].va[s];


	x = x - chunk[ichunk].maxx - pctl->nthick/2;
	if(pctl->y_flip ) {
	    y = y - chunk[ichunk].maxy - pctl->nthick/2;
	} else {
	    y = -y + chunk[ichunk].miny - pctl->nthick/2;
	}

	x *= chunk[ichunk].scale;
	y *= chunk[ichunk].scale;

	x += boxes[ichunk + NCONTROLBOXES].x + 96 * boxes[ichunk+NCONTROLBOXES].w / 100;
	y += boxes[ichunk + NCONTROLBOXES].y + 96 * (boxes[ichunk+NCONTROLBOXES].h/isplit) / 100;

	*xx = x;
	*yy = y;
			
	if(pctl->veloc) {
	   va = boxes[ichunk + NCONTROLBOXES].y + 96 * (boxes[ichunk+NCONTROLBOXES].h) / 100;
	   va -= chunk[ichunk].va[s] * (boxes[ichunk+NCONTROLBOXES].h/isplit) / 
	                                     chunk[ichunk].maxv;
   	*vv = va;
	}
} 

void scale_sample (
		  Boxtype boxes[],
		  Chunktype chunk[],
		  int ichunk,
		  int s,
		  Plotcontrol *pctl,
		  double *xx,
		  double *yy,
		  double *vv)
{
	*vv = 0;
    if(pctl->scale_mode == SCALE_REAL_SETUP) {
         scale_sample_real(boxes, chunk, ichunk, s, pctl->y_flip, xx, yy);
    } else {
         scale_sample_auto(boxes, chunk, ichunk, s, pctl, xx, yy, vv);
    }
}

void bound_y_offset (
		  int *y_offset,
		  Boxtype boxes[],
		  int last_box)
{

	if(*y_offset > boxes[last_box].y - Y_TITLE_AREA) {
		*y_offset = boxes[last_box].y - Y_TITLE_AREA; 
		XBell(dpy,50); 
	}
	if(*y_offset < 0) {
		*y_offset = 0;
		XBell(dpy,50); 
	}
}

void handle_word_clicked (
		  Chunktype chunk[],
		  int ichunk,
		  char current_unipen_file[])
{
    
     char os_line[256];
     char *os_command;
 
                  
     if((os_command = getenv("UPVIEW_COMMAND")) != NULL) {
         sprintf(os_line, "%s %s %d %s &"
                  , os_command
                  , current_unipen_file
                  , ichunk
                  , chunk[ichunk].correct_name);
         fprintf(stdout,"processing chunk: [%s]\n",os_line);
         system(os_line); 
          
     } else {
         sprintf(os_line, "%s %s %d %s &"
                  , "$UPVIEW_COMMAND"
                  , current_unipen_file
                  , ichunk
                  , chunk[ichunk].correct_name);     
         fprintf(stdout,"No environment variable UPVIEW_COMMAND points to\n");
         fprintf(stdout,"a script for processing this chunk:\n%s\n",os_line);
         
     } 
}


char *get_correct_label  (char *segment_label, char *level_name)
{
     char *pret, *chr_ptr;

     pret = strchr (segment_label, '\"');

     if (pret == NULL)
     {
        pret = level_name;
        
     } else {
    	chr_ptr = strchr(pret,'\0') - 1;
    
    	while (chr_ptr > pret && *chr_ptr != '\"')
        {
    	    --chr_ptr;
    	}
	     
        if(chr_ptr != pret)
        {
                   *(chr_ptr+1) = '\0';
       	}
     }
     return(pret);
}
     
/* PostScript Area */
     
void open_postscript (Plotcontrol *pctl)
{
   char Name_PostScript[100];
   
   sprintf(Name_PostScript,"upview-%02d.ps", pctl->N_PostScript++);
   
   fprintf(stdout,"The PostScript PRINT file is named: %s\n"
                 , Name_PostScript);
   
   pctl->F_PostScript = fopen(Name_PostScript,"w");
   
   if(pctl->F_PostScript == NULL) {
       fprintf(stderr,"Error opening file [%s] for PostScript output\n", Name_PostScript);
       exit(1);
   }
   fprintf(pctl->F_PostScript,"%%!PS-Adobe-2.0\n");
   fprintf(pctl->F_PostScript,"%%%%Creator: %s\n", UV_VERSION);
   fprintf(pctl->F_PostScript,"%%%%Title: UNIPEN=%s POSTSCRIPT=%s\n"
                                     , pctl->Title_PostScript,Name_PostScript);
   fprintf(pctl->F_PostScript,"%%%%BoundingBox: %d %d %d %d\n"
                                     , 1-1, 1-1, POST_SQUARE+1, POST_SQUARE+1);
   
   fprintf(pctl->F_PostScript,"0 rotate\n"); 
   fprintf(pctl->F_PostScript,"15 15 translate\n");
   fprintf(pctl->F_PostScript,"/upmat matrix currentmatrix def\n"); 
   fprintf(pctl->F_PostScript,"initclip newpath 0 0 moveto 0 2399 lineto 2399 2399 lineto 2399 0 lineto clip newpath\n"); 
   fprintf(pctl->F_PostScript,"0 0 0 setrgbcolor\n");
   fprintf(pctl->F_PostScript,"%%\n");
   fprintf(pctl->F_PostScript,"/m /moveto load def\n");
   fprintf(pctl->F_PostScript,"/l /lineto load def\n");
   fprintf(pctl->F_PostScript,"/n /newpath load def\n");
   fprintf(pctl->F_PostScript,"/s /stroke load def\n");
   fprintf(pctl->F_PostScript,"%%\n");
   fprintf(pctl->F_PostScript,"0.5 setlinewidth\n");
   fprintf(pctl->F_PostScript,"1 setlinejoin\n");
   fprintf(pctl->F_PostScript,"%%\n");
   fprintf(pctl->F_PostScript,"/Times-Roman findfont 18 scalefont setfont\n");
   fprintf(pctl->F_PostScript,"%%\n");
   fprintf(pctl->F_PostScript,"newpath\n");
   fprintf(pctl->F_PostScript," 1 %d m\n", POST_SQUARE);
   fprintf(pctl->F_PostScript," %d %d l\n", POST_SQUARE, POST_SQUARE);
   fprintf(pctl->F_PostScript," %d 1 l\n", POST_SQUARE);
   fprintf(pctl->F_PostScript," 1 1 l\n");
   fprintf(pctl->F_PostScript," 1 %d l\n", POST_SQUARE);
   fprintf(pctl->F_PostScript,"stroke\n");
   fprintf(pctl->F_PostScript,"%%\n");
   fprintf(pctl->F_PostScript,"15 700 moveto (%s) show\n",pctl->Title_PostScript);
   
}
   
void close_postscript (Plotcontrol *pctl)
{
   fprintf(pctl->F_PostScript,"showpage\n");
   fclose(pctl->F_PostScript);
}

char *make_stripped_label (char *str)
{
	static char cln[100];
	char *beg, *end, *ret;
	
	beg = strchr(str,'"');
	if(beg == NULL) {
		ret = str;
	} else {
		strcpy(cln, beg + 1);
		end = strchr(cln,'"');
		if(end != NULL) {
			*end = ':';
			*(end+1) = (char) 0;
		}
		ret = cln;
	}
	return(ret);
} /* End make_stripped_label() */

int igetfilesize (char filename[])
{
   struct stat file_status_info;

   stat(filename, &file_status_info);
   return(file_status_info.st_size);
} /* End igetfilesize() */

int file_has_changed (char *fname)
{
	static time_t prev_mtime = -1;
	struct stat file_status_info;

	stat(fname, &file_status_info);
	if (prev_mtime!=file_status_info.st_mtime) {
		if (prev_mtime==-1) {
			prev_mtime = file_status_info.st_mtime;
			return 0;
		} else {
			prev_mtime = file_status_info.st_mtime;
			return 1;
		}
	}
	return 0;
}

void finish_file (Boxtype boxes[])
/* Routine to cleanup or perform actions when the current file is left */
{
     mark_boxes(boxes,cBoxStateChanged);
}

/* Draw the control buttons */

void mark_boxes (Boxtype boxes[NBOXES], int     istate)
{
      int i;	
      
      for (i = 0; i < NCONTROLBOXES; i++) {
	    boxes[i].state = istate;
      }
}                   
/* End upview.c */
