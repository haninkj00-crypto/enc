#include "../CODEC.H"
#include "../TABLES/tables.h"
int codec_decode_cp437(const unsigned char *p,unsigned int n,unsigned int *used,U32 *c)
{ if(!n)return 0; *c=cp437_to_unicode[p[0]]; *used=1; return 1; }
