#include <stdio.h>
#include <string.h>
#include "CODEC.H"
#include "CONVERT.H"
#include "ERROR.H"
#include "TABLES/tables.h"
int recovery_file_test(void);
int conversion_file_test(void);

int codec_self_test(void)
{
    unsigned long i;
    U32 c;
    U16 code;
    CP949Entry entry;
    unsigned int n, m;
    unsigned char b[8];
    unsigned char pair[2];
    unsigned char one;
    int rc;

    for (i = 0; i < cp949_forward_count; ++i) {
        entry = cp949_forward_at(i);
        pair[0] = (unsigned char)(entry.code >> 8);
        pair[1] = (unsigned char)entry.code;
        if (codec_decode_cp949(pair, 2, &n, &c) != 1 || n != 2) return 1;
        if (c != entry.scalar) return 2;
        if (!codec_encode_cp949(c, b, &m) || m != 2) return 3;
        code = (U16)(((unsigned int)b[0] << 8) | b[1]);
        if (code != entry.code) return 4;
    }

    {
        unsigned char bad1[] = { 0xC0, 0xAF };
        unsigned char bad2[] = { 0xE0, 0x80, 0x80 };
        unsigned char bad3[] = { 0xF4, 0x90, 0x80, 0x80 };
        if (codec_decode_utf8(bad1, 2, &n, &c) >= 0) return 5;
        if (codec_decode_utf8(bad2, 3, &n, &c) >= 0) return 6;
        if (codec_decode_utf8(bad3, 4, &n, &c) >= 0) return 7;
    }
    {
        unsigned char u8[] = { 0xF0, 0x9F, 0x98, 0x80 };
        if (codec_decode_utf8(u8, 4, &n, &c) != 1 || n != 4 || c != 0x1F600UL) return 8;
        if (!codec_encode_utf8(c, b, &m) || m != 4 || memcmp(b, u8, 4) != 0) return 9;
    }

    for (i = 0; i < 256; ++i) {
        one = (unsigned char)i;
        if (codec_decode_cp437(&one, 1, &n, &c) != 1 || n != 1) return 10;
        if (!codec_encode_cp437(c, b, &m) || m != 1 || b[0] != one) return 11;
        if (cp1252_to_unicode[i] != 0xFFFFFFFFUL) {
            if (codec_decode_cp1252(&one, 1, &n, &c) != 1 || n != 1) return 12;
            if (!codec_encode_cp1252(c, b, &m) || m != 1 || b[0] != one) return 13;
        } else if (codec_decode_cp1252(&one, 1, &n, &c) >= 0) return 14;
    }

    {
        unsigned char u16[] = { 0x3D, 0xD8, 0x00, 0xDE };
        if (codec_decode_utf16le(u16, 4, &n, &c) != 1 || n != 4 || c != 0x1F600UL) return 15;
        if (!codec_encode_utf16le(c, b, &m) || m != 4 || memcmp(b, u16, 4) != 0) return 16;
    }

    if (codec_find_decoder("cP949") == 0 ||
        codec_find_decoder("uTf-8-bOm") == 0 ||
        codec_find_encoder("cp1252") == 0) return 17;

    rc = recovery_file_test();
    if (rc != 0) return rc;
    rc = conversion_file_test();
    if (rc != 0) return rc;
    puts("OK");
    return 0;
}

/* Regression tests for both kinds of 4 KiB recovery boundary. */
int recovery_file_test(void)
{
    FILE *f;
    static unsigned char raw[2], glyph[8], expected[4104], actual[4112];
    unsigned int used, n1, n2, scalar_len, actual_len;
    U32 scalar, recovered;
    unsigned long i;
    unsigned int prefixes[2];
    ConversionError error;
    int test, rc;

    raw[0] = 0xB0;
    raw[1] = 0xA1; /* CP949: 가 */
    if (codec_decode_cp949(raw, 2, &used, &scalar) != 1 || used != 2) return 20;
    if (codec_decode_cp437(raw, 1, &used, &recovered) != 1 || used != 1) return 21;
    if (!codec_encode_utf8(recovered, glyph, &n1)) return 22;
    if (codec_decode_cp437(raw + 1, 1, &used, &recovered) != 1 || used != 1) return 23;
    if (!codec_encode_utf8(recovered, glyph + n1, &n2)) return 24;

    /* 4093: recovered CP949 lead byte is pending at the chunk boundary.
       4095: the first UTF-8 glyph itself is split across the chunk boundary. */
    prefixes[0] = 4093;
    prefixes[1] = 4095;
    for (test = 0; test < 2; ++test) {
        f = fopen("ETRECIN.TMP", "wb");
        if (!f) return 25;
        for (i = 0; i < prefixes[test]; ++i) {
            if (fputc('A', f) == EOF) {
                fclose(f);
                remove("ETRECIN.TMP");
                return 26;
            }
        }
        if (fwrite(glyph, 1, n1 + n2, f) != n1 + n2 ||
            fputc('Z', f) == EOF || fclose(f) != 0) {
            remove("ETRECIN.TMP");
            return 27;
        }
        for (i = 0; i < prefixes[test]; ++i) expected[i] = 'A';
        if (!codec_encode_utf8(scalar, expected + prefixes[test], &scalar_len)) {
            remove("ETRECIN.TMP");
            return 28;
        }
        expected[prefixes[test] + scalar_len] = 'Z';
        rc = convert_file_recovery("UTF-8", "UTF-8", "ETRECIN.TMP", "ETRECOUT.TMP", 0, NULL);
        if (rc != 0) {
            remove("ETRECIN.TMP");
            remove("ETRECOUT.TMP");
            return 29;
        }
        f = fopen("ETRECOUT.TMP", "rb");
        if (!f) {
            remove("ETRECIN.TMP");
            return 30;
        }
        actual_len = (unsigned int)fread(actual, 1, sizeof(actual), f);
        if (ferror(f) || fclose(f) != 0) {
            remove("ETRECIN.TMP");
            remove("ETRECOUT.TMP");
            return 31;
        }
        remove("ETRECIN.TMP");
        remove("ETRECOUT.TMP");
        if (actual_len != prefixes[test] + scalar_len + 1) return 32;
        if (memcmp(actual, expected, prefixes[test] + scalar_len) != 0 ||
            actual[actual_len - 1] != 'Z') return 33;
    }

    /* A lone recovered CP949 lead byte is rejected in strict mode and
       converted to U+FFFD in replacement mode. */
    f = fopen("ETRECIN.TMP", "wb");
    if (!f) return 34;
    if (fwrite(glyph, 1, n1, f) != n1 || fclose(f) != 0) {
        remove("ETRECIN.TMP");
        return 35;
    }
    rc = convert_file_recovery("UTF-8", "UTF-8", "ETRECIN.TMP", "ETRECOUT.TMP", 0, &error);
    if (rc != 14 || error.name == 0 || strcmp(error.name, "INVALID_CP949") != 0) {
        remove("ETRECIN.TMP"); remove("ETRECOUT.TMP"); return 36;
    }
    rc = convert_file_recovery("UTF-8", "UTF-8", "ETRECIN.TMP", "ETRECOUT.TMP", 1, &error);
    if (rc != 0 || error.replaced_recovery != 1) {
        remove("ETRECIN.TMP"); remove("ETRECOUT.TMP"); return 37;
    }
    f = fopen("ETRECOUT.TMP", "rb");
    if (!f) { remove("ETRECIN.TMP"); remove("ETRECOUT.TMP"); return 38; }
    actual_len = (unsigned int)fread(actual, 1, sizeof(actual), f);
    if (ferror(f) || fclose(f) != 0 || actual_len != 3 ||
        actual[0] != 0xEF || actual[1] != 0xBF || actual[2] != 0xBD) {
        remove("ETRECIN.TMP"); remove("ETRECOUT.TMP"); return 39;
    }
    remove("ETRECIN.TMP");
    remove("ETRECOUT.TMP");
    return 0;
}


int conversion_file_test(void)
{
    FILE *f;
    static unsigned char actual[32];
    static const unsigned char bad_utf8[] = { 'A', 0xF0, 0x9F };
    static const unsigned char expected_utf8[] = { 'A', 0xEF, 0xBF, 0xBD };
    static const unsigned char emoji[] = { 0xF0, 0x9F, 0x98, 0x80 };
    static const unsigned char bad_949[] = { 0xB0, 0x20 };
    static const unsigned char expected_949_replace[] = { 0xEF, 0xBF, 0xBD, 0x20 };
    static const unsigned char expected_qmark[] = { '?' };
    unsigned int n;
    ConversionError error;
    int rc;

    remove("ETIN.TMP");
    remove("ETOUT.TMP");
    f = fopen("ETIN.TMP", "wb");
    if (f == 0) return 40;
    if (fwrite(bad_utf8, 1, sizeof(bad_utf8), f) != sizeof(bad_utf8) || fclose(f) != 0) {
        remove("ETIN.TMP");
        return 41;
    }
    rc = convert_file("uTf-8", "UTF-8", "ETIN.TMP", "ETOUT.TMP", 0, &error);
    if (rc != 8 || error.offset != 1 ||
        error.name == 0 || strcmp(error.name, "INVALID_UTF8") != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 42;
    }
    rc = convert_file("uTf-8", "utf-8", "ETIN.TMP", "ETOUT.TMP", 1, &error);
    if (rc != 0 || error.replaced_input != 1) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 43;
    }
    f = fopen("ETOUT.TMP", "rb");
    if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 44; }
    n = (unsigned int)fread(actual, 1, sizeof(actual), f);
    if (ferror(f) || fclose(f) != 0 || n != sizeof(expected_utf8) ||
        memcmp(actual, expected_utf8, sizeof(expected_utf8)) != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 45;
    }

    f = fopen("ETIN.TMP", "wb");
    if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 46; }
    if (fwrite(emoji, 1, sizeof(emoji), f) != sizeof(emoji) || fclose(f) != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 47;
    }
    rc = convert_file("UTF-8", "cp949", "ETIN.TMP", "ETOUT.TMP", 0, &error);
    if (rc != 10 || error.name == 0 || strcmp(error.name, "UNREPRESENTABLE") != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 48;
    }
    rc = convert_file("UTF-8", "Cp949", "ETIN.TMP", "ETOUT.TMP", 1, &error);
    if (rc != 0 || error.replaced_output != 1) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 49;
    }
    f = fopen("ETOUT.TMP", "rb");
    if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 50; }
    n = (unsigned int)fread(actual, 1, sizeof(actual), f);
    if (ferror(f) || fclose(f) != 0 || n != 1 || actual[0] != expected_qmark[0]) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 51;
    }

    f = fopen("ETIN.TMP", "wb");
    if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 52; }
    if (fwrite(bad_949, 1, sizeof(bad_949), f) != sizeof(bad_949) || fclose(f) != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 53;
    }
    rc = convert_file("CP949", "UTF-8", "ETIN.TMP", "ETOUT.TMP", 0, &error);
    if (rc != 9 || error.offset != 0 || error.name == 0 ||
        strcmp(error.name, "INVALID_CP949") != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 54;
    }
    rc = convert_file("cp949", "utf-8", "ETIN.TMP", "ETOUT.TMP", 1, &error);
    if (rc != 0 || error.replaced_input != 1) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 55;
    }
    f = fopen("ETOUT.TMP", "rb");
    if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 56; }
    n = (unsigned int)fread(actual, 1, sizeof(actual), f);
    if (ferror(f) || fclose(f) != 0 || n != sizeof(expected_949_replace) ||
        memcmp(actual, expected_949_replace, sizeof(expected_949_replace)) != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 57;
    }

    rc = convert_file("NOT-AN-ENCODING", "UTF-8", "ETIN.TMP", "ETOUT.TMP", 0, &error);
    if (rc != 2 || error.name == 0 ||
        strcmp(error.name, "UNSUPPORTED_INPUT_ENCODING") != 0) {
        remove("ETIN.TMP"); remove("ETOUT.TMP"); return 58;
    }

    {
        static const unsigned char bom_bad[] = { 0xEF, 0xBB, 0xBF, 'A', 0xF0, 0x9F };
        f = fopen("ETIN.TMP", "wb");
        if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 64; }
        if (fwrite(bom_bad, 1, sizeof(bom_bad), f) != sizeof(bom_bad) || fclose(f) != 0) {
            remove("ETIN.TMP"); remove("ETOUT.TMP"); return 65;
        }
        rc = convert_file("UTF-8", "UTF-8", "ETIN.TMP", "ETOUT.TMP", 0, &error);
        if (rc != 8 || error.offset != 4) {
            remove("ETIN.TMP"); remove("ETOUT.TMP"); return 66;
        }
    }

    {
        static const unsigned char bom_data[] = { 0xEF, 0xBB, 0xBF, 'B' };
        static const unsigned char bom_expected[] = { 0xEF, 0xBB, 0xBF, 'B' };
        f = fopen("ETIN.TMP", "wb");
        if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 59; }
        if (fwrite(bom_data, 1, sizeof(bom_data), f) != sizeof(bom_data) || fclose(f) != 0) {
            remove("ETIN.TMP"); remove("ETOUT.TMP"); return 60;
        }
        rc = convert_file("uTf-8-bOm", "UTF-8-BOM", "ETIN.TMP", "ETOUT.TMP", 0, &error);
        if (rc != 0) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 61; }
        f = fopen("ETOUT.TMP", "rb");
        if (!f) { remove("ETIN.TMP"); remove("ETOUT.TMP"); return 62; }
        n = (unsigned int)fread(actual, 1, sizeof(actual), f);
        if (ferror(f) || fclose(f) != 0 || n != sizeof(bom_expected) ||
            memcmp(actual, bom_expected, sizeof(bom_expected)) != 0) {
            remove("ETIN.TMP"); remove("ETOUT.TMP"); return 63;
        }
    }

    remove("ETIN.TMP");
    remove("ETOUT.TMP");
    return 0;
}
