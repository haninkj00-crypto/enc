#include "../CODEC.H"
#include "../TABLES/tables.h"
int codec_encode_cp949(U32 c,unsigned char*out,unsigned int*n)
{
    unsigned long lo=0,hi=cp949_reverse_count,mid;
    CP949ReverseEntry entry;
    if(c<0x80){out[0]=(unsigned char)c;*n=1;return 1;}
    while(lo<hi){
        mid=lo+(hi-lo)/2;
        entry=cp949_reverse_at(mid);
        if(entry.scalar==c){out[0]=(unsigned char)(entry.code>>8);out[1]=(unsigned char)entry.code;*n=2;return 1;}
        if(entry.scalar<c)lo=mid+1;else hi=mid;
    }
    return 0;
}
