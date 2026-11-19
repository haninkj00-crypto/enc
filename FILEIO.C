#include <stdio.h>
#include <string.h>
#include "CODEC.H"
#include "FILEIO.H"
#ifdef ET_DOS16
#include <fcntl.h>
#include <io.h>
#endif

#ifdef ET_DOS16
#define ET_PATH_MAX 128
static int make_sidecar(const char *path, const char *ext, char *out)
{
    unsigned int n, base, i, dot;
    n = (unsigned int)strlen(path);
    if (n >= ET_PATH_MAX) return 0;
    base = 0;
    for (i = 0; i < n; ++i)
        if (path[i] == '\\' || path[i] == '/' || path[i] == ':') base = i + 1;
    dot = n;
    for (i = base; i < n; ++i) if (path[i] == '.') dot = i;
    if (dot + 5 > ET_PATH_MAX) return 0;
    if (dot == n) {
        strcpy(out, path);
        strcat(out, ext);
    } else {
        memcpy(out, path, dot);
        out[dot] = '\0';
        strcat(out, ext);
    }
    return 1;
}

static int path_equal_dos(const char *a, const char *b)
{
    unsigned char ca, cb;
    while (*a && *b) {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if (ca >= 'a' && ca <= 'z') ca = (unsigned char)(ca - 'a' + 'A');
        if (cb >= 'a' && cb <= 'z') cb = (unsigned char)(cb - 'a' + 'A');
        if (ca != cb) return 0;
    }
    return *a == *b;
}

static int find_unused_sidecar(const char *path, char prefix, char *out)
{
    char ext[5];
    FILE *probe;
    unsigned int i;
    ext[0] = '.';
    ext[1] = prefix;
    ext[4] = '\0';
    for (i = 0; i < 100; ++i) {
        ext[2] = (char)('0' + i / 10);
        ext[3] = (char)('0' + i % 10);
        if (!make_sidecar(path, ext, out)) return 0;
        if (path_equal_dos(path, out)) continue;
        probe = fopen(out, "rb");
        if (!probe) return 1;
        fclose(probe);
    }
    return 0;
}
#endif

int io_write_all(FILE *f, const unsigned char *p, unsigned int n)
{
    return fwrite(p, 1, n, f) == n;
}

int io_output_bom(FILE *f, const char *name)
{
    unsigned char b[3];
    unsigned int n;
    n = 0;
    if (codec_name_equal(name, "UTF-8-BOM")) {
        b[0] = 0xEF; b[1] = 0xBB; b[2] = 0xBF; n = 3;
    } else if (codec_name_equal(name, "UTF-16LE-BOM")) {
        b[0] = 0xFF; b[1] = 0xFE; n = 2;
    } else if (codec_name_equal(name, "UTF-16BE-BOM")) {
        b[0] = 0xFE; b[1] = 0xFF; n = 2;
    }
    return n == 0 ? 1 : io_write_all(f, b, n);
}

int io_input_bom(FILE *f, const char *name,
                 unsigned char *pending, unsigned int *plen,
                 unsigned long *skipped)
{
    unsigned char got[3];
    unsigned int n, i, need;
    int required, match, ch;
    *plen = 0;
    if (skipped != NULL) *skipped = 0;
    need = 0;
    required = 0;
    if (codec_name_equal(name, "UTF-8") || codec_name_equal(name, "UTF-8-BOM")) {
        need = 3;
        if (codec_name_equal(name, "UTF-8-BOM")) required = 1;
    } else if (codec_name_equal(name, "UTF-16LE") ||
               codec_name_equal(name, "UTF-16LE-BOM")) {
        need = 2;
        if (codec_name_equal(name, "UTF-16LE-BOM")) required = 1;
    } else if (codec_name_equal(name, "UTF-16BE") ||
               codec_name_equal(name, "UTF-16BE-BOM")) {
        need = 2;
        if (codec_name_equal(name, "UTF-16BE-BOM")) required = 1;
    } else return 1;

    n = 0;
    while (n < need) {
        ch = fgetc(f);
        if (ch == EOF) break;
        got[n++] = (unsigned char)ch;
    }
    match = 0;
    if (n == need) {
        if (need == 3)
            match = got[0] == 0xEF && got[1] == 0xBB && got[2] == 0xBF;
        else if (codec_name_equal(name, "UTF-16BE") ||
                 codec_name_equal(name, "UTF-16BE-BOM"))
            match = got[0] == 0xFE && got[1] == 0xFF;
        else match = got[0] == 0xFF && got[1] == 0xFE;
    }
    if (required && !match) {
        for (i = 0; i < n; ++i) pending[i] = got[i];
        *plen = n;
        return 0;
    }
    if (!match) {
        for (i = 0; i < n; ++i) pending[i] = got[i];
        *plen = n;
    } else if (skipped != NULL) *skipped = need;
    return 1;
}

int io_open_pair(const char *inpath, const char *outpath,
                 FILE **in, FILE **out, char *temp)
{
    if (inpath == NULL) {
#ifdef ET_DOS16
        if (setmode(fileno(stdin), O_BINARY) == -1) return 7;
#endif
        *in = stdin;
    } else {
        *in = fopen(inpath, "rb");
        if (*in == NULL) return 4;
    }

    if (outpath == NULL) {
#ifdef ET_DOS16
        if (setmode(fileno(stdout), O_BINARY) == -1) {
            if (*in != stdin) fclose(*in);
            return 11;
        }
#endif
        *out = stdout;
        temp[0] = '\0';
    } else {
#ifdef ET_DOS16
        if (!find_unused_sidecar(outpath, 'T', temp)) {
            if (*in != stdin) fclose(*in);
            return 5;
        }
#else
        if (strlen(outpath) + 5 >= 1024) {
            if (*in != stdin) fclose(*in);
            return 3;
        }
        strcpy(temp, outpath);
        strcat(temp, ".tmp");
#endif
        *out = fopen(temp, "wb");
        if (*out == NULL) {
            if (*in != stdin) fclose(*in);
            return 5;
        }
    }

    return 0;
}

int io_finish_pair(FILE *in, FILE *out, const char *temp,
                   const char *outpath, int rc)
{
#ifdef ET_DOS16
    char backup[ET_PATH_MAX];
#else
    char backup[1024];
#endif
    if (in != stdin && fclose(in) != 0 && rc == 0) rc = 7;
    if (out == stdout) {
        if (fflush(stdout) != 0 && rc == 0) rc = 11;
    } else if (fclose(out) != 0 && rc == 0) rc = 11;

    if (outpath != NULL && rc == 0 && rename(temp, outpath) != 0) {
#ifdef ET_DOS16
        if (!find_unused_sidecar(outpath, 'B', backup)) rc = 11;
        else if (rename(outpath, backup) != 0) rc = 11;
        else if (rename(temp, outpath) != 0) {
            rename(backup, outpath);
            rc = 11;
        } else remove(backup);
#else
        if (strlen(outpath) + 5 >= sizeof(backup)) rc = 11;
        else {
            strcpy(backup, outpath);
            strcat(backup, ".bak");
            if (rename(outpath, backup) != 0) rc = 11;
            else if (rename(temp, outpath) != 0) {
                rename(backup, outpath);
                rc = 11;
            } else remove(backup);
        }
#endif
    }
    if (outpath != NULL && rc != 0) remove(temp);
    return rc;
}
