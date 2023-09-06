
typedef struct {
     double base;        /* base line in y: modus (base line) */
     double mean;        /* base line in y: ymean */
     double sd;          /* sd of y around mean   */
     double sdb;         /* sd of y around base   */
     double ymin;        /* minimum of y          */
     double ymax;        /* maximum of y          */
     int n;              /* number of contourpoints encountered */
     int ndb;            /* number of contours assumed in baseline */
     int ncont;          /* number of contours */

     double *yhist;      /* histogram of y values */
     int nbins;          /* number of bins in yhistogram */
     double *ypeaklist;  /* y of peaks, sorted in decreasing likelihood p(y) */
     int *ipeakpos;      /* position of peaks */
     int npeaks;         /* number of peaks */
} Yfont;
         
typedef struct {
     double xmin;
     double xmax;
     double ymin;
     double ymax;
     
     double xmean;
     double ymean;
     
     int irightmost;
} Box;     

#define NPEAKS_MAX 3 /* max assumed number of lines in a line strip */
