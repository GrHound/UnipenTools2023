/**************************************************************************
*                                                                         *
*  UNIPEN PROJECT  (upview.h)                                             *
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pwd.h>

#include <uplib.h>
#include <upsiglib.h>

/* 

Using program should call:

X_initialize()
X_getevent()

*/

#define  keyColor_Box         "Box"
#define  keyColor_ControlBox  "Button"
#define  keyColor_FlashBox    "FlashBox"
#define  keyColor_CrossBox    "CrossBox"
#define  keyColor_SetupBox    "SetupBox"
#define  keyColor_Ink         "Ink"
#define  keyColor_Air         "Air"
#define  keyColor_Name        "Text"
#define  keyColor_ControlName "ButtonText"
#define  keyColor_BackGround  "BackGround"
 
#define  keyButtonFontName    "ButtonFontName"
#define  keySmallFontName     "SmallFontName"
#define  keyBigFontName       "BigFontName"
#define  keyHugeFontName      "HugeFontName"


#define	DRAW_AREA_WIDTH	   	770
#define	DRAW_AREA_HEIGHT	770
#define BUTTONCOLW_INIT          80

int window_width  = DRAW_AREA_WIDTH + BUTTONCOLW_INIT;
int window_paint  = DRAW_AREA_WIDTH;
int window_height = DRAW_AREA_HEIGHT;

#define WINDW           window_width
#define WINDH           window_height

#define BUTTONCOLW   ((int) (0.5 + (double) window_width * BUTTONCOLW_INIT / (double) (DRAW_AREA_WIDTH + BUTTONCOLW_INIT)))
#define BUTTONROWH   ((int) (0.5 + (double) window_height / 11.8))

#define Y_TITLE_AREA     40

#define BUTW             (8 * BUTTONCOLW) / 10
#define BUTH             (8 * BUTTONROWH) / 10
#define BUTD             ((BUTTONCOLW - BUTW)/2)



#define POST_SQUARE 560
#define POST_SCALE 0.8
#define POST_THICK 0.5

#define INITIAL_GESTURE_THICKNESS 6

#define cBoxStateChanged          1
#define cBoxStateUnchanged        0

#define cStateFileToBeOpened      1
#define cStateFileToBeClosed      2
#define cStateFileIsOpen          3
#define cStateFileIsClosed        4
#define cStateFileToBeFreed       5
#define cStateFileIsFreed         6



#define INFTX           999999
#define INFTY           999999

#define	MAXCHUNKS	10000
#define NCHARMAX        256

#define	NCONTROLBOXES	18
#define	NBOXES		(NCONTROLBOXES + MAXCHUNKS)
#define ASPECTW         2

#define MAX_HLINES        200

#define SCALE_PER_CHUNK  1
#define SCALE_ON_BIGGEST 2
#define SCALE_REAL_SETUP 3
#define LAB_SCALE_ON_BIGGEST "Same Scale"
#define LAB_SCALE_PER_CHUNK  "Opt. Scale"
#define LAB_SCALE_REAL_SETUP "Tablet Scale"


#define USR_QUIT      1
#define USR_NEXTF     2
#define USR_PREVF     3
#define USR_ZOOM      4
#define USR_HIERARCHY 5 

#define LAB_QUIT       "Quit"
#define LAB_NEXTF      "NextFile"
#define LAB_PREVF      "PrevFile"
#define LAB_UP         "Up"
#define LAB_DOWN       "Down"
#define LAB_MORE_INK   "MoreInk"
#define LAB_LESS_INK   "LessInk"
#define LAB_ZOOM_IN    "ZoomIn"
#define LAB_ZOOM_OUT   "ZoomOut"
#define LAB_HIER_UP    "HierUp"
#define LAB_HIER_DOWN  "HierDown"
#define LAB_PRINT      "Print"

#define LAB_NOVELOC    "Add 'Velocity'"
#define LAB_VELOC      "No Velocity"

#define LAB_GESTURE    "Tapered"
#define LAB_NOGEST     "No Tapering"

#define LAB_OPENED_LINES "Open Lines"
#define LAB_FILLED_LINES "Filled Lines"

#define LAB_ROUNDED_LINES "Sample Dots"
#define LAB_JAGGED_LINES  "No Sample Dots"

#define LAB_SHOW_PENUP    "Show PenUp"
#define LAB_NOSHOW_PENUP    "Hidden PenUp"

#define BUT_PREVF       0
#define BUT_NEXTF       1
#define BUT_QUIT        2
#define BUT_DOWN        3
#define BUT_UP          4
#define BUT_MORE_INK    5
#define BUT_LESS_INK    6
#define BUT_ZOOM_IN     7
#define BUT_ZOOM_OUT    8
#define BUT_HIER_UP     9
#define BUT_HIER_DOWN  10
#define BUT_PRINT      11
#define BUT_SCALE      12
#define BUT_VELOC      13
#define BUT_GESTURE    14
#define BUT_OPEN       15
#define BUT_ROUNDED    16
#define BUT_PENUP      17

#define MAX_INK_THICKNESS   200
#define MAX_COLS_REQUESTED   50
#define MAX_LEVEL_REQUESTED  10 /*Just to prevent ridiculous levels: in fact there is no limit */

/* Macros */

#define ZERO(x)		memset(x, 0, sizeof(x))
#define INBOX(b,v,z)	((v > b.x) && (v < b.x + b.w) && (z > b.y) && (z < b.y + b.h))
/* v and z are pointer x and y, but these letters would be substituted in the 
   macro for the field names as well */

#define VISIBLE_BOX(i) ((boxes[i].y>=y_offset)&&(boxes[i].y+boxes[i].h<(y_offset+(layout->nrows+2)*layout->lboxheight)))

#define Dprintf   if(DEB)fprintf
#define Dflush    if(DEB)fflush
#define DEB 0

typedef struct {
    int ncolumns;
    int nrows;
    int lboxwidth;
    int lboxheight;
    int windw;
    int windh;
    int maxwholechunk;
    int upper_left_chunk;
    int lower_right_chunk;
    } Layout;
   

typedef struct {
	int    x, y, w, h;
	char   descr[NCHARMAX];
	int    ldescr;
	char   va_title[NCHARMAX];
	int    lva_title;
	int    state;
	upBool visible;
} Boxtype;

typedef struct {
        char   file_name[NCHARMAX];     /* File from which chunk is read */
        int    lfile_name;              /* Number of characters in file_name */
	int    nxyz;                    /* number of samples */
	int    maxt;                    /* maximum number of samples in a group of chunks */
	int    minx;                    /* Extrema this chunk */
	int    maxx;
	int    miny;
	int    maxy;
	int    maxv;
	sigSignal *signal_stream;             /* The x,y,z */
	int *va;                              /* and derived |D(k)| channel */
	float  scale;                   /* current scaling factor */
	char   correct_name[NCHARMAX];  /* label of this chunk of handwriting */
	int    lcorrect_name;           /* Number of characters in correct_name */
   int    is_drawn;
   int    is_scaled;
	} Chunktype;
	
typedef struct {
	int nthick;
	int gesture_mode;
	int scale_mode;
	int veloc;
	int y_flip;
	int open_lines;
	int rounded_lines;
	int show_penup;
	int do_poll;
	int no_labels;

	int Do_PostScript;
	int Do_PostScript_BoxTitle;
	int N_PostScript;
	FILE *F_PostScript;
	char Title_PostScript[100];
        
	int    x_dim;                   /* Box info concerning real-life setup */
	int    x_dim_given;
	int    y_dim;
	int    y_dim_given;
	int    h_line_nlines;
	int    h_lines[MAX_HLINES];
	int    h_line_given;

        } Plotcontrol;

static char ButtonFontName[256] =   "fixed";

static char SmallFontName[256] =  "-ADOBE-NEW CENTURY SCHOOLBOOK-BOLD-R-NORMAL--*-100-*-*-P-*";

static char BigFontName[256] =    "-ADOBE-COURIER-BOLD-R-NORMAL--*-120-*-*-M-*";

static char HugeFontName[256] =   "-ADOBE-COURIER-BOLD-R-NORMAL--*-240-*-*-M-*";

/* defined in upview.c */

/* extern int window_width,window_height; */

Display *dpy;
Window window1, window2;
GC gcBox;
GC gcFlashBox;
GC gcCrossBox;
GC gcControlBox;
GC gcSetupBox;
GC gcInk;
GC gcAir;
GC gcName;
GC gcFileName;
GC gcControlName;

Font BigFont;
Font SmallFont;
Font ButtonFont;

#define BELL ((char) 7)

#define NCHRCOL 128
static char Color_Box[NCHRCOL];
static char Color_ControlBox[NCHRCOL];
static char Color_FlashBox[NCHRCOL];
static char Color_CrossBox[NCHRCOL];
static char Color_SetupBox[NCHRCOL];
static char Color_Ink[NCHRCOL];
static char Color_Air[NCHRCOL];
static char Color_Name[NCHRCOL];
static char Color_ControlName[NCHRCOL];
static char Color_BackGround[NCHRCOL];

Screen *screen;
int n, state = 0;

/* relevant UNIPEN keywords */

#define cKeyXDim  ".X_DIM"
#define cKeyYDim  ".Y_DIM"
#define cKeyHLINE ".H_LINE"

/* Prototypes */

	void	X_initialize();
	void	doCreateWindows();
	void	doCreateGraphicsContexts();
	static	unsigned long doDefineColor();
	void	doWMHints();
	void	doMapWindows();
	int	X_getevent();
	void    some_default_setting();
	void    get_default_setting();
	void    save_default_setting();
	void    getHomeDir();
	void    read_a_keyword();
	
void X_initialize(ww, wh, wx, wy, wheader)
int ww, wh, wx, wy;
char *wheader;
{
  get_default_setting();

  dpy = XOpenDisplay(0);
  if (!dpy)
  {
    fputs("Display not opened!\n", stderr);
    exit(-1);
  }

  screen = XDefaultScreenOfDisplay(dpy);
  if (!screen)
  {
    fputs("No screen!\n", stderr);
    exit (-1);
  }

  doCreateWindows(ww, wh, wx, wy);

  doCreateGraphicsContexts();

  doWMHints(ww, wh, wx, wy, wheader);

  doMapWindows();

}


void doCreateWindows(ww, wh, wx, wy)
int ww, wh, wx, wy;
{
  XSetWindowAttributes xswa;

  xswa.event_mask = ExposureMask | ButtonPressMask | KeyPressMask | 
  ButtonReleaseMask;
  xswa.background_pixel = doDefineColor(Color_BackGround,0);
  xswa.override_redirect = 1;

  window1 = XCreateWindow(dpy, XRootWindowOfScreen(screen) 
              , wx, wy, ww, wh, 4
              , XDefaultDepthOfScreen(screen), InputOutput
              , XDefaultVisualOfScreen(screen)
              , CWEventMask | CWBackPixel, &xswa);
  
}


void doCreateGraphicsContexts()
{
/* clipping rectangle added by Loe (brute force rules) */
	Region region = XCreateRegion();
	XRectangle rectangle;

  XGCValues xgcvBox;
  XGCValues xgcvControlBox;
  XGCValues xgcvFlashBox;
  XGCValues xgcvCrossBox;
  XGCValues xgcvSetupBox;
  XGCValues xgcvFileName;
  XGCValues xgcvName;
  XGCValues xgcvControlName;
  XGCValues xgcvInk;
  XGCValues xgcvAir;
  static Pixmap clp;

  rectangle.x = 0;
  rectangle.y = 0;
  rectangle.width = window_paint;
  rectangle.height = window_height;
  XUnionRectWithRegion (&rectangle,region,region);

  SmallFont = XLoadFont (dpy, SmallFontName);
  ButtonFont = XLoadFont (dpy, ButtonFontName);
  BigFont = XLoadFont (dpy, BigFontName);

  xgcvBox.background = doDefineColor(Color_BackGround,0);
  xgcvBox.foreground = doDefineColor(Color_Box,1);
  gcBox = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvBox);
  XSetFont (dpy, gcBox, ButtonFont);
  
  xgcvControlBox.background = doDefineColor(Color_BackGround,0);
  xgcvControlBox.foreground = doDefineColor(Color_ControlBox,1);
  gcControlBox = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvControlBox);
  XSetFillStyle(dpy, gcControlBox, FillSolid);
  
  xgcvFlashBox.background = doDefineColor(Color_BackGround,0);
  xgcvFlashBox.foreground = doDefineColor(Color_FlashBox,1);
  gcFlashBox = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvFlashBox);
  
  xgcvCrossBox.background = doDefineColor(Color_BackGround,0);
  xgcvCrossBox.foreground = doDefineColor(Color_CrossBox,1);
  gcCrossBox = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvCrossBox);

  xgcvSetupBox.background = doDefineColor(Color_BackGround,0);
  xgcvSetupBox.foreground = doDefineColor(Color_SetupBox,1);
  gcSetupBox = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvSetupBox);
  
  xgcvFileName.background = doDefineColor(Color_BackGround,0);
  xgcvFileName.foreground = doDefineColor(Color_Name,1);
  gcFileName = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvFileName);
  
	xgcvName.background = doDefineColor(Color_BackGround,0);
  xgcvName.foreground = doDefineColor(Color_Name,1);
  xgcvName.clip_mask = clp;

  gcName = XCreateGC(dpy, window1, GCForeground 
                                    | GCBackground
                                    | GCClipMask, &xgcvName);
  XSetFont (dpy, gcName, SmallFont);
	XSetRegion (dpy,gcName,region);
   
  xgcvControlName.background = doDefineColor(Color_BackGround,0);
  xgcvControlName.foreground = doDefineColor(Color_ControlName,1);
  xgcvControlName.clip_mask = clp;
  
  gcControlName = XCreateGC(dpy, window1, GCForeground 
                                           | GCBackground 
                                           | GCClipMask, &xgcvControlName);
  XSetFont (dpy, gcControlName, BigFont);
 
  xgcvInk.background = doDefineColor(Color_BackGround,0);
  xgcvInk.foreground = doDefineColor(Color_Ink,1);
  gcInk = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvInk);
  XSetFillStyle(dpy, gcInk, FillSolid);
	XSetRegion (dpy,gcInk,region);

  xgcvAir.background = doDefineColor(Color_BackGround,0);
  xgcvAir.foreground = doDefineColor(Color_Air,1);
  gcAir = XCreateGC(dpy, window1, GCForeground | GCBackground, &xgcvAir);
	XSetRegion (dpy,gcAir,region);
}	

static unsigned long doDefineColor(color_string,n)
char color_string[];
int n;
{
  unsigned long pixel;
  XColor exact_color, screen_color;
  char filename[100];

  if ((XDefaultVisualOfScreen(screen))->class == PseudoColor
        ||
      (XDefaultVisualOfScreen(screen))->class == DirectColor)
  {
    if (XAllocNamedColor(dpy, XDefaultColormapOfScreen(screen), color_string, &screen_color, &exact_color))
      return screen_color.pixel;
    else
      getHomeDir(filename);
      fprintf(stderr,"Color [%s] unknown, check or delete your '.upviewrc' file\n", color_string);
      exit(1);
  }
  else
  {
/*    fputs("Not a color device!\n", stderr);
*/
    switch(n)
    {
      case 1 :
        pixel = XBlackPixelOfScreen(screen);
      break;

      case 0 :
        pixel = XWhitePixelOfScreen(screen);
      break;

      default:
        pixel = XBlackPixelOfScreen(screen);
      break;
    }
  }

  return (pixel);
}

void doWMHints(ww, wh, wx, wy, wheader)
int ww, wh, wx, wy;
char *wheader;
{
  XSizeHints xsh;

  xsh.x = wx;
  xsh.y = wy;
  xsh.width = ww;
  xsh.height = wh;
  xsh.min_width = ww;
  xsh.min_height = wh;
  xsh.max_width = ww;
  xsh.max_height = wh;
  xsh.flags = PPosition | PSize | PMinSize | PMaxSize;

  /* XSetNormalHints(dpy, window1, &xsh); X11R3 and X11R4 */
  
  XSetWMNormalHints(dpy, window1, &xsh); /* X11R5 */

  XStoreName(dpy, window1, wheader);
}

void doMapWindows()
{
  XMapWindow(dpy, window1);

}

int X_getevent(event)
XEvent *event;
{
  XNextEvent(dpy, event);
  return(event->type);
}

/* Environment and Resource stuff */

void getHomeDir( dest )
char *dest;
{
    int uid;
    extern char *getenv();
    struct passwd *pw;
    register char *ptr;

    if ((ptr = getenv("HOME")) != NULL) {
        (void) strcpy(dest, ptr);

    } else {
        if ((ptr = getenv("USER")) != NULL) {
            pw = getpwnam(ptr);
        } else {
            uid = getuid();
            pw = getpwuid(uid);
        }
        if (pw) {
            (void) strcpy(dest, pw->pw_dir);
        } else {
            *dest = '\0';
        }
    }
}

void get_default_setting()
{
	char answer[100];
	char filename[512];
	FILE *ctl;
 
	getHomeDir(filename);
	strcat(filename,"/.upviewrc");
 
	ctl = fopen(filename,"r");
 
	if(ctl == NULL) {
		printf ("no '%s' found, ok to create one? (y/n): ",filename);
		fflush(stdout);
		scanf("%s",answer);
		if (answer[0]!='y')
			exit(1);
		fprintf(stderr,"Initializing first %s\n", filename);
		some_default_setting();
		save_default_setting();
	} else {
		read_a_keyword(ctl,keyColor_Box,         Color_Box);
		read_a_keyword(ctl,keyColor_ControlBox,  Color_ControlBox);
		read_a_keyword(ctl,keyColor_FlashBox,    Color_FlashBox);
		read_a_keyword(ctl,keyColor_CrossBox,    Color_CrossBox);
		read_a_keyword(ctl,keyColor_SetupBox,    Color_SetupBox);
		read_a_keyword(ctl,keyColor_Ink,         Color_Ink);
		read_a_keyword(ctl,keyColor_Air,         Color_Air);
		read_a_keyword(ctl,keyColor_Name,        Color_Name);
		read_a_keyword(ctl,keyColor_ControlName, Color_ControlName);
		read_a_keyword(ctl,keyColor_BackGround,  Color_BackGround);
		read_a_keyword(ctl,keySmallFontName, ButtonFontName);
		read_a_keyword(ctl,keySmallFontName, SmallFontName);
		read_a_keyword(ctl,keyBigFontName,   BigFontName);
		read_a_keyword(ctl,keyHugeFontName,  HugeFontName);
		fclose(ctl);
	}
}

void save_default_setting()
{
       char filename[512];
       FILE *ctl;
       
       getHomeDir(filename);
       strcat(filename,"/.upviewrc");
       
       ctl = fopen(filename,"w");
       if(ctl == NULL) {
              fprintf(stderr,"Error saving upview settings in file %s\n", filename);
       } else {
              fprintf(ctl,"%s=%s\n",  keyColor_Box,        Color_Box);
              fprintf(ctl,"%s=%s\n",  keyColor_ControlBox, Color_ControlBox);
              fprintf(ctl,"%s=%s\n",  keyColor_FlashBox,   Color_FlashBox);
              fprintf(ctl,"%s=%s\n",  keyColor_CrossBox,   Color_CrossBox);
              fprintf(ctl,"%s=%s\n",  keyColor_SetupBox,   Color_SetupBox);
              fprintf(ctl,"%s=%s\n",  keyColor_Ink,        Color_Ink);
              fprintf(ctl,"%s=%s\n",  keyColor_Air,        Color_Air);
              fprintf(ctl,"%s=%s\n",  keyColor_Name,       Color_Name);
              fprintf(ctl,"%s=%s\n",  keyColor_ControlName,Color_ControlName);
              fprintf(ctl,"%s=%s\n",  keyColor_BackGround, Color_BackGround);
              
              fprintf(ctl,"%s=%s\n",  keyButtonFontName,    ButtonFontName);
              fprintf(ctl,"%s=%s\n",  keySmallFontName,    SmallFontName);
              fprintf(ctl,"%s=%s\n",  keyBigFontName,      BigFontName);
              fprintf(ctl,"%s=%s\n",  keyHugeFontName,     HugeFontName);
  
              fclose(ctl);
       }
}

void some_default_setting()
{
    strcpy(Color_Box,         "dark slate blue");
    strcpy(Color_ControlBox,  "black");
    strcpy(Color_FlashBox,    "blue");
    strcpy(Color_CrossBox,    "medium slate blue");
    strcpy(Color_SetupBox,    "yellow");
    strcpy(Color_Ink,         "firebrick");
    strcpy(Color_Air,         "yellow");
    strcpy(Color_Name,        "dark slate blue");
    strcpy(Color_ControlName, "dark slate blue" );
    strcpy(Color_BackGround,  "light grey");
    
    strcpy(ButtonFontName,
            "fixed");

    strcpy(SmallFontName,
            "-ADOBE-NEW CENTURY SCHOOLBOOK-BOLD-R-NORMAL--*-100-*-*-P-*");

    strcpy(BigFontName,
            "-ADOBE-COURIER-BOLD-R-NORMAL--*-120-*-*-M-*");

    strcpy(HugeFontName,
            "-ADOBE-COURIER-BOLD-R-NORMAL--*-240-*-*-M-*");
    
    
}

void read_a_keyword(fp,key,string)
FILE *fp;
char key[];
char string[NCHRCOL];
{
      char this_key[100];
      
      strcpy(this_key,"");
      
      while(strcmp(this_key,key) != 0) {
      	      if(fscanf(fp,"%[^=]=%[^\n]\n",this_key,string) != 2) {
      	      	  fprintf(stderr,"Error finding keyword [%s] in ~/.upviewrc\n"
      	      	                ,key);
      	      	  fclose(fp);
      	      	  exit(1);
      	      }
      }
      rewind(fp);
}
