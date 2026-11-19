#include "../CODEC.H"
#include "../TABLES/tables.h"
int codec_encode_cp437(U32 c,unsigned char*out,unsigned int*n){unsigned int i;for(i=0;i<256;i++)if(cp437_to_unicode[i]==c){out[0]=(unsigned char)i;*n=1;return 1;}return 0;}
