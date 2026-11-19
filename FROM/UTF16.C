#include "../CODEC.H"
static int decode16(const unsigned char*p,unsigned int n,unsigned int*u,U32*c,int be)
{ U32 x,y; if(n<2)return 0; x=be?((U32)p[0]<<8)|p[1]:((U32)p[1]<<8)|p[0]; if(x>=0xD800&&x<=0xDBFF){if(n<4)return 0;y=be?((U32)p[2]<<8)|p[3]:((U32)p[3]<<8)|p[2];if(y<0xDC00||y>0xDFFF)return -1;*c=0x10000UL+((x-0xD800UL)<<10)+(y-0xDC00UL);*u=4;return 1;} if(x>=0xDC00&&x<=0xDFFF)return -1;*c=x;*u=2;return 1; }
int codec_decode_utf16le(const unsigned char*p,unsigned int n,unsigned int*u,U32*c){return decode16(p,n,u,c,0);}
int codec_decode_utf16be(const unsigned char*p,unsigned int n,unsigned int*u,U32*c){return decode16(p,n,u,c,1);}
