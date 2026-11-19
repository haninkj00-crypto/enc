#include "../CODEC.H"
static int encode16(U32 c,unsigned char*out,unsigned int*n,int be){U16 a,b;if(c>0x10FFFF||(c>=0xD800&&c<=0xDFFF))return 0;if(c<0x10000){a=(U16)c;*n=2;}else{c-=0x10000;a=(U16)(0xD800+(c>>10));b=(U16)(0xDC00+(c&1023));*n=4;if(be){out[0]=(unsigned char)(a>>8);out[1]=(unsigned char)a;out[2]=(unsigned char)(b>>8);out[3]=(unsigned char)b;}else{out[0]=(unsigned char)a;out[1]=(unsigned char)(a>>8);out[2]=(unsigned char)b;out[3]=(unsigned char)(b>>8);}return 1;}if(be){out[0]=(unsigned char)(a>>8);out[1]=(unsigned char)a;}else{out[0]=(unsigned char)a;out[1]=(unsigned char)(a>>8);}return 1;}
int codec_encode_utf16le(U32 c,unsigned char*out,unsigned int*n){return encode16(c,out,n,0);}
int codec_encode_utf16be(U32 c,unsigned char*out,unsigned int*n){return encode16(c,out,n,1);}
