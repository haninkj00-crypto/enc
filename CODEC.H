#ifndef CODEC_H
#define CODEC_H
/* C89-compatible scalar codec interface. */
#ifdef ET_DOS16
# define ET_FAR _far
#else
# define ET_FAR
#endif
typedef unsigned long U32;
typedef unsigned short U16;
typedef struct { U16 code; U32 scalar; } CP949Entry;
typedef struct { U32 scalar; U16 code; } CP949ReverseEntry;
/* Decoder: 1=one scalar, 0=need more bytes, -1=invalid sequence. */
typedef int (*DecodeOne)(const unsigned char *, unsigned int,
                         unsigned int *, U32 *);
/* Encoder: 1=success, 0=unrepresentable/invalid scalar. */
typedef int (*EncodeOne)(U32, unsigned char *, unsigned int *);
typedef struct { const char *name; DecodeOne decode; } Decoder;
typedef struct { const char *name; EncodeOne encode; } Encoder;
typedef struct {
    const char *name;
    const char *detail;
    unsigned long offset;
    unsigned char bytes[4];
    unsigned int byte_count;
    U32 scalar;
    int has_scalar;
    unsigned long replaced_input;
    unsigned long replaced_output;
    unsigned long replaced_recovery;
} ConversionError;
int codec_name_equal(const char *a, const char *b);
const Decoder *codec_find_decoder(const char *name);
const Encoder *codec_find_encoder(const char *name);
int codec_decode_cp437(const unsigned char *, unsigned int, unsigned int *, U32 *);
int codec_decode_cp1252(const unsigned char *, unsigned int, unsigned int *, U32 *);
int codec_decode_cp949(const unsigned char *, unsigned int, unsigned int *, U32 *);
int codec_decode_utf8(const unsigned char *, unsigned int, unsigned int *, U32 *);
int codec_decode_utf16le(const unsigned char *, unsigned int, unsigned int *, U32 *);
int codec_decode_utf16be(const unsigned char *, unsigned int, unsigned int *, U32 *);
int codec_encode_cp437(U32, unsigned char *, unsigned int *);
int codec_encode_cp1252(U32, unsigned char *, unsigned int *);
int codec_encode_cp949(U32, unsigned char *, unsigned int *);
int codec_encode_utf8(U32, unsigned char *, unsigned int *);
int codec_encode_utf16le(U32, unsigned char *, unsigned int *);
int codec_encode_utf16be(U32, unsigned char *, unsigned int *);
#endif
