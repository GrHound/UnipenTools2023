#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xmu/Drawing.h>
#endif

#include <math.h>

#include "bitmap_routines.h"

static int x_hot = -1;
static int y_hot = -1;

#ifdef USE_X11
unsigned char *read_bitmap (int *w, int *h, char *fname)
{
	unsigned char *bitmap;
	unsigned int width,height;

    if (XmuReadBitmapDataFromFile(fname,&width,&height,&bitmap,&x_hot, &y_hot)!=BitmapSuccess) {
		fprintf (stderr, "unable to read bitmap from file '%s'\n",fname);
		exit (1);
	}
	*w = (int) width;
	*h = (int) height;

	return bitmap;
}
#endif

char **convert_bitmap2data (unsigned char *bitmap, int width, int height)
{
	int i,j;
	unsigned char *dp = bitmap;
	static unsigned char masktable[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	int idx,padded;
	char **data;

	padded = ((width & 7) != 0);
	data = (char **) calloc (height,sizeof(char *));

	for (j=0;j<height;j++) {
		data[j] = (char *) calloc (width,sizeof(char));
		for (i=0;i<width;i++) {
			idx = (i & 7);
			if (*dp & masktable[idx])
				data[j][i] = 1;
			if (idx == 7) dp++;
		}
		if (padded) dp++;
	}
	return data;
}

/* I found basename is not on all platforms ... */
static char *basename (char *path)
{
	char *p,*ptr;

	p = path;
	while ((ptr=strchr(p,'/'))!=NULL)
		p = ptr;
	if (p[0]=='/')
		return p+1;
	else
		return p;
}

char *fname2name (char *fname)
{
	char *ptr;
	static char result[256];

	ptr = basename(fname);
	strcpy(result,ptr);
	if ((ptr=strchr(result,'.'))!=NULL)
		ptr[0] = '\0';
	return result;
}

void write_bitmap (int width, int height, char **data, char *fname)
{
	FILE *fp;
	char *name;
	int i, j;
	char *cp;
	int last_character;
	int bytes_per_scanline = 0;
	struct _scan_list {
		int allocated;
		int used;
		unsigned char *scanlines;
		struct _scan_list *next;
	} *head = NULL, *slist;
	static unsigned char masktable[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	int padded = 0;

	if ((fp=fopen(fname,"w"))==NULL) {
		fprintf (stderr,"unable to open '%s' for output!\n",fname);
		exit(1);
	}
	name = fname2name(fname);

#define NTOALLOC 16
#define NewSList() \
	slist = (struct _scan_list *) calloc (1, sizeof *slist); \
	if (!slist) { \
		fprintf (stderr, "unable to allocate scan list\n"); \
		return; \
	} \
	slist->allocated = NTOALLOC * bytes_per_scanline; \
	slist->scanlines = (unsigned char *) calloc(slist->allocated, 1); \
	if (!slist->scanlines) { \
		fprintf (stderr, "unable to allocate char array\n"); \
		return; \
	} \
	slist->used = 0; \
	slist->next = NULL; 

	padded = ((width & 7) != 0);
	bytes_per_scanline = (width + 7) / 8;
	NewSList ();
	head = slist;

	for (j=0;j<height;j++) {
		cp = data[j];
		if (slist->used + 1 >= slist->allocated) {
			struct _scan_list *old = slist;
			NewSList ();
			old->next = slist;
		}

		for (i=0;i<width;i++) {
			int ind = (i & 7);
			int on = 0;
			if (cp[i]>0)
				on = 1;
			if (on)
				slist->scanlines[slist->used] |= masktable[ind];
			if (ind == 7) slist->used++;
		}
		if (padded) slist->used++;
	}

	/* output the scan lines */
	fprintf (fp,"#define %s_width %d\n", name, width);
	fprintf (fp,"#define %s_height %d\n", name, height);
	if (x_hot >= 0) fprintf (fp,"#define %s_x_hot %d\n", name, x_hot);
	if (y_hot >= 0) fprintf (fp,"#define %s_y_hot %d\n", name, y_hot);
	fprintf (fp,"\n");
	fprintf (fp,"static char %s_bits[] = {\n", name);

	j = 0;
	last_character = height * bytes_per_scanline - 1;
	for (slist = head; slist; slist = slist->next) {
		for (i = 0; i < slist->used; i++) {
			fprintf (fp,"  0x%02x", slist->scanlines[i]);
			if (j != last_character)
				fprintf (fp,",");
			if ((j % 12) == 11)
				fprintf (fp,"\n");
			j++;
		}
	}
	fprintf (fp," };\n");
	fclose(fp);
}
