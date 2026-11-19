#ifndef CONVERT_H
#define CONVERT_H
#include "CODEC.H"
/* NULL input/output path selects stdin/stdout. */
int convert_file(const char *from, const char *to,
                 const char *inpath, const char *outpath, int replace,
                 ConversionError *error);
int convert_file_recovery(const char *from, const char *to,
                          const char *inpath, const char *outpath, int replace,
                          ConversionError *error);
#endif
