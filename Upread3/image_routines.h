#ifndef _IMAGE_ROUTINES
#define _IMAGE_ROUTINES

#define NUL ((char) 0)

#define MINPRES .25

#define MIN_BRUSH 2

#define MAX_COORDINATES_IN_HWR 2048
#define MAX_PIXELS_IN_IMG      100000

#define GREY_PIXEL 255
#define ADD_PIXEL -999
#define RED    -1
#define WHITE  -2
#define GREEN  -3
#define RANCOL -4

#define O_XBM 0
#define O_PPM 1
#define O_PGM 2

#define INK_SOLID            0
#define INK_FOUNTAIN         1
#define INK_FOUNTAINWETPAPER 2
#define INK_CALLIGRAPHIC_45  3
#define INK_SPRAY_DISK_50    4
#define INK_BALLPOINT        5

#define MAXBRUSH 51

typedef int ** IMG;
typedef unsigned char Pix;
typedef struct {
   int r;
   int g;
   int b;
} ColPix;

extern void write_image (char *fname, IMG image, int w, int h, int frmt);

extern void write_merged_image (IMG image, int w, int h, char *fname);
extern void write_data_ppm (char *fname, IMG image, int w, int h);
extern void write_data_pgm (char *fname, IMG image, int w, int h);
extern void write_bitmap_data (char *fname, IMG image, int w, int h);
extern void brush (IMG im, int w, int h, int x, int y, int col, double a_brush[MAXBRUSH][MAXBRUSH], int BRUSH, double v, int inkmodel);
extern void draw_line_in_image (IMG image, int w, int h, int x1, int y1, int x2, int y2, int col, int BRUSH, double v, int inkmodel);
extern void make_rectangles (IMG image, int x0, int y0, int N, int width, int height);
extern void plot_allo_in_image (IMG image, int w, int h, int n, int *x, int *y, int *z, int col, int BRUSH, int inkmodel);
extern void float_plot_allo_in_image (IMG image, int w, int h, int n
	, float *x, float *y, float *z, int col, int BRUSH, int filled, int nsmo, int inkmodel);
extern void normalize_allo_xy (int n, int *x, int *y, double *xo, double *yo);
extern void float_normalize_allo_xy (int n, float *x, float *y, float *xo, float *yo);
extern void scale_allo (int n, double *xi, double *yi, int *x, int *y, int width, int height, int margin, double range);
extern void float_scale_allo (int n, float *xi, float *yi, float *zi
	, float *x, float *y, float *z, int width, int height, int margin, double range, int scalemode);
extern int determine_brush (int xmin,int xmax,int ymin,int ymax,int width,int height);
extern int count_pixels_in_sub_image (IMG image, double *X, double *Y, int x0, int y0, int N, int width);

extern void visualize_image (IMG , int, int, int);
extern void clear_image (IMG , int, int);

extern IMG create_image(int width, int height);
extern void read_data_p3 (FILE *fp, IMG *image, int *w, int *h, double *Gmax);
extern void read_data_p5 (FILE *fp, IMG *image, int *w, int *h, double *Gmax);
extern void read_data_p6 (FILE *fp, IMG *image, int *w, int *h, double *Gmax);
extern void read_data (char *fname, IMG *image, int *w, int *h, double *Gmax);
 
#endif

