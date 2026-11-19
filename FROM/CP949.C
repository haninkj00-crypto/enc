#include "../CODEC.H"
#include "../TABLES/tables.h"
int codec_decode_cp949(const unsigned char *p,unsigned int n,unsigned int *used,U32 *c)
{
    unsigned long lo,hi,mid;
    U16 key;
    CP949Entry entry;
    if(!n)return 0;
    if(p[0]<0x80){*c=p[0];*used=1;return 1;}
    if(n<2)return 0;
    key=(U16)(((unsigned int)p[0]<<8)|p[1]);
    lo=0;hi=cp949_forward_count;
    while(lo<hi){
        mid=lo+(hi-lo)/2;
        entry=cp949_forward_at(mid);
        if(entry.code==key){*c=entry.scalar;*used=2;return 1;}
        if(entry.code<key)lo=mid+1;else hi=mid;
    }
    return -1;
}
