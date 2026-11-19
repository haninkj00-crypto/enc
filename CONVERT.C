#include <stdio.h>
#include <string.h>
#include "CODEC.H"
#include "CONVERT.H"
#include "FILEIO.H"
#include "ERROR.H"
#define IO_SIZE 4096
#define OUT_SIZE 4096

int convert_file(const char *from, const char *to, const char *inpath,
                 const char *outpath, int replace, ConversionError *error)
{
    const Decoder *d;
    const Encoder *e;
    FILE *in, *out;
    static unsigned char ibuf[IO_SIZE + 4], obuf[OUT_SIZE], encbuf[8];
    unsigned int have, got, pos, used, wrote, op, detail_n;
    unsigned long base, scalar_index;
    U32 scalar;
    int rc, eof, replaced;
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
    e = codec_find_encoder(to);
    if (!e) {
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
    scalar_index = 0;
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
            rc = d->decode(ibuf + pos, got - pos, &used, &scalar);
            if (rc == 0) {
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
                rc = 1;
            } else if (rc < 0) {
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
                rc = 1;
            }

            pos += used;
            rc = 0;
            replaced = 0;
            if (!conversion_encode(e->encode, scalar, encbuf, &wrote,
                                   replace, &replaced)) {
                conversion_error_scalar(error, "UNREPRESENTABLE", to,
                                        scalar_index, scalar);
                rc = 10;
                break;
            }
            if (replaced && error != NULL) ++error->replaced_output;
            ++scalar_index;
            if (op + wrote > OUT_SIZE) {
                if (!io_write_all(out, obuf, op)) { rc = 11; break; }
                op = 0;
            }
            memcpy(obuf + op, encbuf, wrote);
            op += wrote;
        }
        if (rc == 8 || rc == 9 || rc == 10 || rc == 11 || rc == 7) {
            if (op && !io_write_all(out, obuf, op)) rc = 11;
            break;
        }
        if (op && !io_write_all(out, obuf, op)) { rc = 11; break; }
        base += pos;
        have = got - pos;
        if (have > 4) { rc = 12; break; }
        if (have) memmove(ibuf, ibuf + pos, have);
        if (eof && have) {
            /* A replace-mode incomplete tail is consumed in the loop above. */
            if (replace) continue;
            conversion_error_bytes(error, conversion_input_error_name(from),
                                   from, base, ibuf, have);
            rc = 8;
            break;
        }
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
