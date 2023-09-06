#ifndef _DIR_WRITER_IDS_
#define _DIR_WRITER_IDS_

#include <uplib.h>

#ifndef WRITER_ID_FILE
#define WRITER_ID_FILE "/vol/realtime5/louis/newhome/nici/resources/WRITERCODES/writer_identifications"
#endif

 
typedef struct {
	char level[64];
	int seg_idx;
	char seg_name[64];
	int idx;
	char *writer_name;
	int dataset;
	char *path;
} WriterIdentification;

extern void parse_writercode (char *writer_code, WriterIdentification *writer_id);
extern char *writerid_2_upfile (WriterIdentification *writer_id);

extern int  read_writer_identifications (char *fname);
extern int updirFilename2WriterCode (char *fname);
extern char *updirEntry2WriterCode (tUPEntry *entry, tUPEntry **entries
   , int nentries, int wcode);
   
extern void updirectFname_to_writer_id(char *fname, char *codestr);

#endif
