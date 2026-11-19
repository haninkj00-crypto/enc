#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
int io_write_all(FILE *f, const unsigned char *p, unsigned int n);
int io_input_bom(FILE *f, const char *name, unsigned char *pending,
                  unsigned int *plen, unsigned long *skipped);
/* A NULL input path means stdin; a NULL output path means stdout. */
int io_open_pair(const char *inpath, const char *outpath,
                 FILE **in, FILE **out, char *temp);
int io_output_bom(FILE *f, const char *name);
int io_finish_pair(FILE *in, FILE *out, const char *temp,
                   const char *outpath, int rc);
#endif
