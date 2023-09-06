#ifndef _OUTPUT_IMAGE_
#define _OUTPUT_IMAGE_

#include "image_routines.h"

extern void init_output_image (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo);

extern int output_image (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);


#endif
