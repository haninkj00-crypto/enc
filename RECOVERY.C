#include <stdio.h>
#include <string.h>
#include "CODEC.H"
#include "CONVERT.H"
#include "FILEIO.H"
#include "ERROR.H"
#define IO_SIZE 4096
#define OUT_SIZE 4096

static int recovery_emit(const Encoder *e, FILE *out, unsigned char *obuf,
                         unsigned int *op, U32 scalar, int replace,
                         ConversionError *error, unsigned long output_index)
{
    unsigned char bytes[8];
    unsigned int n;
    int did_replace;
    if (!conversion_encode(e->encode, scalar, bytes, &n, replace, &did_replace)) {
        conversion_error_scalar(error, "UNREPRESENTABLE", e->name,
                                output_index, scalar);
        return 10;
    }
    if (did_replace && error != NULL) ++error->replaced_output;
    if (*op + n > OUT_SIZE) {
        if (!io_write_all(out, obuf, *op)) return 11;
        *op = 0;
    }
    memcpy(obuf + *op, bytes, n);
    *op += n;
    return 0;
}

static int recovery_feed_byte(const Decoder *k, const Encoder *e, FILE *out,
                              unsigned char *obuf, unsigned int *op,
                              unsigned char one, int *mhave,
                              unsigned char *pending_byte,
                              unsigned long *pending_offset,
                              unsigned long *recovered_offset,
                              unsigned long *output_index, int replace,
                              ConversionError *error)
{
    unsigned char pair[2];
    unsigned int used;
    U32 korean;
    int kr, rc;

    for (;;) {
        if (*mhave == 0) {
            kr = k->decode(&one, 1, &used, &korean);
            if (kr == 0) {
                *pending_byte = one;
                *pending_offset = *recovered_offset;
                *mhave = 1;
                ++*recovered_offset;
                return 0;
            }
            if (kr < 0) {
                if (!replace) {
                    conversion_error_bytes(error, "INVALID_CP949", "CP949 recovery bytes",
                                           *recovered_offset, &one, 1);
                    return 14;
                }
                ++*recovered_offset;
                if (error != NULL) ++error->replaced_recovery;
                rc = recovery_emit(e, out, obuf, op, 0xFFFDUL, replace,
                                   error, *output_index);
                if (rc) return rc;
                ++*output_index;
                return 0;
            }
            ++*recovered_offset;
            rc = recovery_emit(e, out, obuf, op, korean, replace,
                               error, *output_index);
            if (rc) return rc;
            ++*output_index;
            return 0;
        }

        pair[0] = *pending_byte;
        pair[1] = one;
        kr = k->decode(pair, 2, &used, &korean);
        if (kr == 1) {
            *mhave = 0;
            ++*recovered_offset;
            rc = recovery_emit(e, out, obuf, op, korean, replace,
                               error, *output_index);
            if (rc) return rc;
            ++*output_index;
            return 0;
        }
        if (!replace) {
            conversion_error_bytes(error, "INVALID_CP949", "CP949 recovery bytes",
                                   *pending_offset, pair, 2);
            return 14;
        }

        /* The pending lead byte is bad; replace it, then reprocess this byte. */
        *mhave = 0;
        if (error != NULL) ++error->replaced_recovery;
        rc = recovery_emit(e, out, obuf, op, 0xFFFDUL, replace,
                           error, *output_index);
        if (rc) return rc;
        ++*output_index;
    }
}

int convert_file_recovery(const char *from, const char *to,
                          const char *inpath, const char *outpath, int replace,
                          ConversionError *error)
{
    const Decoder *d, *k;
    const Encoder *g, *e;
    FILE *in, *out;
    static unsigned char ibuf[IO_SIZE + 4], obuf[OUT_SIZE], midbuf[2];
    unsigned char pending_byte;
    unsigned int have, got, pos, used, wrote, op, detail_n;
    unsigned long base, source_index, recovered_offset, pending_offset, output_index;
    U32 scalar;
    int rc, eof, dr;
#ifdef ET_DOS16
    char temp[128];
#else
    char temp[1024];
#endif

    conversion_error_clear(error);
    d = codec_find_decoder(from);
    if (!d) {
        conversion_error_simple(error, "UNSUPPORTED_INPUT_ENCODING", from);
        return 2;
    }
    g = codec_find_encoder("CP437");
    k = codec_find_decoder("CP949");
    e = codec_find_encoder(to);
    if (!g || !k || !e) {
        conversion_error_simple(error, "UNSUPPORTED_OUTPUT_ENCODING", to);
        return 2;
    }
    rc = io_open_pair(inpath, outpath, &in, &out, temp);
    if (rc) {
        if (rc == 4) conversion_error_simple(error, "INPUT_OPEN_FAILED", inpath ? inpath : "stdin");
        else if (rc == 5 || rc == 3) conversion_error_simple(error, "OUTPUT_OPEN_FAILED", outpath ? outpath : "stdout");
        else if (rc == 7) conversion_error_simple(error, "STDIN_BINARY_MODE_FAILED", "stdin");
        else if (rc == 11) conversion_error_simple(error, "STDOUT_BINARY_MODE_FAILED", "stdout");
        else conversion_error_simple(error, "OUTPUT_WRITE_FAILED", outpath ? outpath : "stdout");
        return rc;
    }
    if (!io_input_bom(in, from, ibuf, &have, &base)) {
        conversion_error_bytes(error, "BOM_REQUIRED", from, 0, ibuf, have);
        return io_finish_pair(in, out, temp, outpath, 6);
    }
    if (!io_output_bom(out, to)) {
        conversion_error_simple(error, "OUTPUT_WRITE_FAILED", outpath ? outpath : "stdout");
        return io_finish_pair(in, out, temp, outpath, 11);
    }

    eof = 0;
    rc = 0;
    source_index = 0;
    recovered_offset = 0;
    pending_offset = 0;
    output_index = 0;
    op = 0;
    {
        int mhave;
        mhave = 0;
        while (!eof || have) {
            got = (unsigned int)fread(ibuf + have, 1, IO_SIZE, in);
            if (got < IO_SIZE) {
                if (ferror(in)) { rc = 7; break; }
                eof = 1;
            }
            got += have;
            pos = 0;
            op = 0;
            while (pos < got) {
                dr = d->decode(ibuf + pos, got - pos, &used, &scalar);
                if (dr == 0) {
                    if (!eof) break;
                    if (!replace) {
                        detail_n = got - pos;
                        conversion_error_bytes(error, conversion_input_error_name(from),
                                               from, base + pos, ibuf + pos, detail_n);
                        rc = 8;
                        break;
                    }
                    used = conversion_incomplete_consume(from, got - pos);
                    if (used == 0) used = 1;
                    scalar = 0xFFFDUL;
                    if (error != NULL) ++error->replaced_input;
                } else if (dr < 0) {
                    if (!replace) {
                        detail_n = got - pos;
                        conversion_error_bytes(error, conversion_input_error_name(from),
                                               from, base + pos, ibuf + pos, detail_n);
                        rc = 9;
                        break;
                    }
                    used = conversion_invalid_consume(from, got - pos);
                    scalar = 0xFFFDUL;
                    if (error != NULL) ++error->replaced_input;
                }
                pos += used;
                if (!g->encode(scalar, midbuf, &wrote) || wrote != 1) {
                    if (!replace) {
                        conversion_error_scalar(error, "CP437_REVERSE", "CP437 glyph table",
                                                source_index, scalar);
                        rc = 13;
                        break;
                    }
                    midbuf[0] = (unsigned char)'?';
                    if (error != NULL) ++error->replaced_recovery;
                }
                ++source_index;
                rc = recovery_feed_byte(k, e, out, obuf, &op, midbuf[0],
                                        &mhave, &pending_byte, &pending_offset,
                                        &recovered_offset, &output_index,
                                        replace, error);
                if (rc) break;
            }
            if (rc == 8 || rc == 9 || rc == 10 || rc == 11 ||
                rc == 13 || rc == 14 || rc == 7) {
                if (op && !io_write_all(out, obuf, op)) rc = 11;
                break;
            }
            if (op && !io_write_all(out, obuf, op)) { rc = 11; break; }
            op = 0;
            base += pos;
            have = got - pos;
            if (have > 4) { rc = 12; break; }
            if (have) memmove(ibuf, ibuf + pos, have);
            if (eof && have && !replace) {
                conversion_error_bytes(error, conversion_input_error_name(from),
                                       from, base, ibuf, have);
                rc = 8;
                break;
            }
        }
        if (rc == 0 && mhave) {
            if (!replace) {
                conversion_error_bytes(error, "INVALID_CP949", "CP949 recovery bytes",
                                       pending_offset, &pending_byte, 1);
                rc = 14;
            } else {
                if (error != NULL) ++error->replaced_recovery;
                rc = recovery_emit(e, out, obuf, &op, 0xFFFDUL, replace,
                                   error, output_index);
                if (rc == 0) ++output_index;
            }
        }
        if (rc == 0 && op && !io_write_all(out, obuf, op)) rc = 11;
    }
    if (rc == 7) conversion_error_simple(error, "INPUT_READ_FAILED", inpath ? inpath : "stdin");
    else if (rc == 11) conversion_error_simple(error, "OUTPUT_WRITE_FAILED", outpath ? outpath : "stdout");
    rc = io_finish_pair(in, out, temp, outpath, rc);
    if (rc == 7 && error != NULL && error->name == NULL)
        conversion_error_simple(error, "INPUT_CLOSE_FAILED", inpath ? inpath : "stdin");
    else if (rc == 11 && error != NULL && error->name == NULL)
        conversion_error_simple(error, "OUTPUT_COMMIT_FAILED", outpath ? outpath : "stdout");
    return rc;
}
