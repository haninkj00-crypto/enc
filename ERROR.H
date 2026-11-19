#ifndef ERROR_H
#define ERROR_H
#include "CODEC.H"
void conversion_error_clear(ConversionError *error);
void conversion_error_bytes(ConversionError *error, const char *name,
                            const char *detail, unsigned long offset,
                            const unsigned char *bytes, unsigned int byte_count);
void conversion_error_scalar(ConversionError *error, const char *name,
                             const char *detail, unsigned long offset, U32 scalar);
void conversion_error_simple(ConversionError *error, const char *name,
                             const char *detail);
const char *conversion_input_error_name(const char *encoding);
unsigned int conversion_invalid_consume(const char *encoding,
                                        unsigned int available);
unsigned int conversion_incomplete_consume(const char *encoding,
                                            unsigned int available);
int conversion_encode(EncodeOne encoder, U32 scalar, unsigned char *out,
                      unsigned int *written, int replace, int *was_replaced);
#endif
