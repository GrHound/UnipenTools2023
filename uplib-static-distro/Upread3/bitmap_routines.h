#ifndef _BITMAP_ROUTINES_
#define _BITMAP_ROUTINES_

unsigned char *read_bitmap (int *w, int *h, char *fname);

char **convert_bitmap2data (unsigned char *bitmap, int width, int height);

char *fname2name (char *fname);

void write_bitmap (int width, int height, char **data, char *fname);

#endif
