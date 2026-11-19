#include <stddef.h>
#include "ERROR.H"

static int encoding_starts(const char *name, const char *prefix)
{
    unsigned char a, b;
    while (*prefix) {
        if (!*name) return 0;
        a = (unsigned char)*name++;
        b = (unsigned char)*prefix++;
        if (a >= 'a' && a <= 'z') a = (unsigned char)(a - 'a' + 'A');
        if (b >= 'a' && b <= 'z') b = (unsigned char)(b - 'a' + 'A');
        if (a != b) return 0;
    }
    return 1;
}

void conversion_error_clear(ConversionError *e)
{
    if (e == NULL) return;
    e->name = NULL;
    e->detail = NULL;
    e->offset = 0;
    e->byte_count = 0;
    e->scalar = 0;
    e->has_scalar = 0;
    e->replaced_input = 0;
    e->replaced_output = 0;
    e->replaced_recovery = 0;
}

void conversion_error_simple(ConversionError *e, const char *name,
                             const char *detail)
{
    if (e == NULL) return;
    e->name = name;
    e->detail = detail;
    e->offset = 0;
    e->byte_count = 0;
    e->has_scalar = 0;
}

void conversion_error_bytes(ConversionError *e, const char *name,
                            const char *detail, unsigned long offset,
                            const unsigned char *p, unsigned int n)
{
    unsigned int i;
    if (e == NULL) return;
    e->name = name;
    e->detail = detail;
    e->offset = offset;
    e->byte_count = n > 4 ? 4 : n;
    for (i = 0; i < e->byte_count; ++i) e->bytes[i] = p[i];
    e->has_scalar = 0;
}

void conversion_error_scalar(ConversionError *e, const char *name,
                             const char *detail, unsigned long offset, U32 scalar)
{
    if (e == NULL) return;
    e->name = name;
    e->detail = detail;
    e->offset = offset;
    e->byte_count = 0;
    e->scalar = scalar;
    e->has_scalar = 1;
}

const char *conversion_input_error_name(const char *encoding)
{
    if (encoding_starts(encoding, "UTF-8")) return "INVALID_UTF8";
    if (encoding_starts(encoding, "UTF-16")) return "INVALID_UTF16";
    if (codec_name_equal(encoding, "CP949")) return "INVALID_CP949";
    if (codec_name_equal(encoding, "CP1252")) return "INVALID_CP1252";
    return "INVALID_INPUT";
}

unsigned int conversion_invalid_consume(const char *encoding,
                                        unsigned int available)
{
    if (available == 0) return 0;
    if (encoding_starts(encoding, "UTF-16"))
        return available >= 2 ? 2 : 1;
    return 1;
}

unsigned int conversion_incomplete_consume(const char *encoding,
                                            unsigned int available)
{
    if (available == 0) return 0;
    if (encoding_starts(encoding, "UTF-16"))
        return available >= 2 ? 2 : 1;
    /* A truncated UTF-8 prefix is one malformed subsequence. */
    return available;
}

int conversion_encode(EncodeOne encoder, U32 scalar, unsigned char *out,
                      unsigned int *written, int replace, int *was_replaced)
{
    int local_replaced;
    if (was_replaced != NULL) *was_replaced = 0;
    if (encoder(scalar, out, written)) return 1;
    if (!replace) return 0;
    if (encoder(0xFFFDUL, out, written) || encoder((U32)'?', out, written)) {
        local_replaced = 1;
        if (was_replaced != NULL) *was_replaced = local_replaced;
        return 1;
    }
    return 0;
}
