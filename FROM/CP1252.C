#include "../CODEC.H"
#include "../TABLES/tables.h"
int codec_decode_cp1252(const unsigned char *p,unsigned int n,unsigned int *used,U32 *c)
{ if(!n)return 0; *c=cp1252_to_unicode[p[0]]; if(*c==0xFFFFFFFFUL)return -1; *used=1; return 1; }
