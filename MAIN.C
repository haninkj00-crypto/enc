#include <stdio.h>
#include <string.h>
#include "CODEC.H"
#include "CONVERT.H"
#include "ERROR.H"

static int arg_equal(const char *a, const char *b)
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

static int path_equal(const char *a, const char *b)
{
#ifdef ET_DOS16
    return arg_equal(a, b);
#else
    return strcmp(a, b) == 0;
#endif
}

static void usage(FILE *stream)
{
    fprintf(stream, "Usage: ENC /F FROM /T TO [/I INPUT] [/O OUTPUT] [/REPLACE]\n");
    fprintf(stream, "       ENC /F UTF-8 /T CP949 /I IN.TXT /O OUT.TXT\n");
    fprintf(stream, "       ENC /F UTF-8 /T UTF-8 /MODE RECOVERY /MISINTERPRET CP437 /REAL CP949\n");
    fprintf(stream, "       Omit /I for standard input; omit /O for standard output.\n");
    fprintf(stream, "       /REPLACE substitutes invalid input and unrepresentable characters.\n");
    fprintf(stream, "       /ERRORS STRICT|REPLACE is equivalent to /STRICT or /REPLACE.\n");
    fprintf(stream, "       ENC /TEST runs the built-in self-test.\n");
}

static void print_error(int rc, const ConversionError *e)
{
    unsigned int i;
    const char *name;
    name = (e != NULL) ? e->name : NULL;

    if (name != NULL && strcmp(name, "UNSUPPORTED_INPUT_ENCODING") == 0) {
        fprintf(stderr, "Error: unsupported input encoding '%s'.\n", e->detail ? e->detail : "");
        return;
    }
    if (name != NULL && strcmp(name, "UNSUPPORTED_OUTPUT_ENCODING") == 0) {
        fprintf(stderr, "Error: unsupported output encoding '%s'.\n", e->detail ? e->detail : "");
        return;
    }
    if (name != NULL && strcmp(name, "UNREPRESENTABLE") == 0) {
        fprintf(stderr, "Error: U+%04lX at character index %lu cannot be represented in %s.\n",
                e->scalar, e->offset, e->detail ? e->detail : "output encoding");
        return;
    }
    if (name != NULL && strcmp(name, "INPUT_OPEN_FAILED") == 0) {
        fprintf(stderr, "Error: cannot open input '%s'.\n", e->detail ? e->detail : "");
        return;
    }
    if (name != NULL && strcmp(name, "OUTPUT_OPEN_FAILED") == 0) {
        fprintf(stderr, "Error: cannot create output '%s'.\n", e->detail ? e->detail : "");
        return;
    }
    if (name != NULL && strcmp(name, "INPUT_READ_FAILED") == 0) {
        fprintf(stderr, "Error: read failed on '%s'.\n", e->detail ? e->detail : "stdin");
        return;
    }
    if (name != NULL && strcmp(name, "OUTPUT_WRITE_FAILED") == 0) {
        fprintf(stderr, "Error: write failed on '%s'.\n", e->detail ? e->detail : "stdout");
        return;
    }
    if (name != NULL && strcmp(name, "OUTPUT_COMMIT_FAILED") == 0) {
        fprintf(stderr, "Error: could not install completed output '%s'.\n", e->detail ? e->detail : "");
        return;
    }
    if (name != NULL && strcmp(name, "STDIN_BINARY_MODE_FAILED") == 0) {
        fprintf(stderr, "Error: could not set standard input to binary mode.\n");
        return;
    }
    if (name != NULL && strcmp(name, "STDOUT_BINARY_MODE_FAILED") == 0) {
        fprintf(stderr, "Error: could not set standard output to binary mode.\n");
        return;
    }
    if (name != NULL && strcmp(name, "BOM_REQUIRED") == 0) {
        fprintf(stderr, "Error: required BOM for %s is missing or invalid at input byte 0.",
                e->detail ? e->detail : "encoding");
    } else if (name != NULL && strncmp(name, "INVALID_", 8) == 0) {
        if (e->detail != NULL && strcmp(e->detail, "CP949 recovery bytes") == 0)
            fprintf(stderr, "Error: invalid reconstructed CP949 sequence at recovered byte offset %lu.", e->offset);
        else
            fprintf(stderr, "Error: invalid byte sequence for %s at input byte offset %lu.",
                    e->detail ? e->detail : name, e->offset);
    } else if (name != NULL) {
        fprintf(stderr, "Error: %s", name);
        if (e->detail != NULL) fprintf(stderr, " (%s)", e->detail);
    } else {
        fprintf(stderr, "Error: conversion failed (code %d)", rc);
    }

    if (e != NULL && e->byte_count != 0) {
        fprintf(stderr, " Bytes:");
        for (i = 0; i < e->byte_count; ++i)
            fprintf(stderr, " %02X", (unsigned int)e->bytes[i]);
    }
    if (e != NULL && e->has_scalar)
        fprintf(stderr, " Codepoint: U+%04lX", e->scalar);
    fputc('\n', stderr);
}

int main(int argc, char **argv)
{
    const char *from, *to, *input, *output, *mode, *mis, *real;
    const char *value;
    int i, rc, replace, policy_seen, test_seen, help_seen;
    int seen_from, seen_to, seen_input, seen_output, seen_mode, seen_mis, seen_real;
    ConversionError error;

    from = to = input = output = mode = mis = real = NULL;
    replace = 0;
    policy_seen = test_seen = help_seen = 0;
    seen_from = seen_to = seen_input = seen_output = 0;
    seen_mode = seen_mis = seen_real = 0;

    for (i = 1; i < argc; ++i) {
        value = argv[i];
        if (arg_equal(value, "/TEST")) {
            if (test_seen) { fprintf(stderr, "Error: duplicate /TEST.\n"); return 1; }
            test_seen = 1;
            continue;
        }
        if (arg_equal(value, "/HELP") || arg_equal(value, "/?")) {
            help_seen = 1;
            continue;
        }
        if (arg_equal(value, "/REPLACE")) {
            if (policy_seen && !replace) { fprintf(stderr, "Error: conflicting error policies.\n"); return 1; }
            policy_seen = 1;
            replace = 1;
            continue;
        }
        if (arg_equal(value, "/STRICT")) {
            if (policy_seen && replace) { fprintf(stderr, "Error: conflicting error policies.\n"); return 1; }
            policy_seen = 1;
            replace = 0;
            continue;
        }

        if (arg_equal(value, "/F") || arg_equal(value, "/FROM")) {
            if (seen_from || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /F value.\n"); return 1; }
            from = argv[++i]; seen_from = 1;
        } else if (arg_equal(value, "/T") || arg_equal(value, "/TO")) {
            if (seen_to || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /T value.\n"); return 1; }
            to = argv[++i]; seen_to = 1;
        } else if (arg_equal(value, "/I") || arg_equal(value, "/INPUT")) {
            if (seen_input || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /I value.\n"); return 1; }
            input = argv[++i]; seen_input = 1;
        } else if (arg_equal(value, "/O") || arg_equal(value, "/OUTPUT")) {
            if (seen_output || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /O value.\n"); return 1; }
            output = argv[++i]; seen_output = 1;
        } else if (arg_equal(value, "/MODE")) {
            if (seen_mode || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /MODE value.\n"); return 1; }
            mode = argv[++i]; seen_mode = 1;
        } else if (arg_equal(value, "/MISINTERPRET")) {
            if (seen_mis || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /MISINTERPRET value.\n"); return 1; }
            mis = argv[++i]; seen_mis = 1;
        } else if (arg_equal(value, "/REAL")) {
            if (seen_real || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /REAL value.\n"); return 1; }
            real = argv[++i]; seen_real = 1;
        } else if (arg_equal(value, "/ERRORS")) {
            if (policy_seen || i + 1 >= argc) { fprintf(stderr, "Error: missing or duplicate /ERRORS value.\n"); return 1; }
            value = argv[++i];
            if (arg_equal(value, "REPLACE")) replace = 1;
            else if (arg_equal(value, "STRICT")) replace = 0;
            else { fprintf(stderr, "Error: unknown error policy '%s'.\n", value); return 1; }
            policy_seen = 1;
        } else {
            fprintf(stderr, "Error: unknown switch or unexpected argument '%s'.\n", value);
            usage(stderr);
            return 1;
        }
    }

    if (help_seen) { usage(stdout); return 0; }
    if (test_seen) {
        extern int codec_self_test(void);
        if (argc != 2) { fprintf(stderr, "Error: /TEST must be used alone.\n"); return 1; }
        return codec_self_test();
    }
    if (!seen_from || !seen_to) {
        fprintf(stderr, "Error: both /F and /T are required.\n");
        usage(stderr);
        return 1;
    }
    if (mode != NULL && !arg_equal(mode, "NORMAL") && !arg_equal(mode, "RECOVERY")) {
        fprintf(stderr, "Error: unsupported mode '%s'.\n", mode);
        return 1;
    }
    if ((mis != NULL || real != NULL) && (mode == NULL || !arg_equal(mode, "RECOVERY"))) {
        fprintf(stderr, "Error: /MISINTERPRET and /REAL require /MODE RECOVERY.\n");
        return 1;
    }
    if (mode != NULL && arg_equal(mode, "RECOVERY")) {
        if (mis == NULL || real == NULL || !arg_equal(mis, "CP437") || !arg_equal(real, "CP949")) {
            fprintf(stderr, "Error: recovery mode requires /MISINTERPRET CP437 /REAL CP949.\n");
            return 1;
        }
    }
    if (input != NULL && output != NULL && path_equal(input, output)) {
        fprintf(stderr, "Error: input and output paths must be different.\n");
        return 1;
    }

    conversion_error_clear(&error);
    if (mode != NULL && arg_equal(mode, "RECOVERY"))
        rc = convert_file_recovery(from, to, input, output, replace, &error);
    else
        rc = convert_file(from, to, input, output, replace, &error);

    if (rc != 0) {
        print_error(rc, &error);
        return rc;
    }
    if (error.replaced_input || error.replaced_output || error.replaced_recovery) {
        fprintf(stderr, "Replacement summary: input=%lu, output=%lu, recovery=%lu.\n",
                error.replaced_input, error.replaced_output, error.replaced_recovery);
    }
    return 0;
}
