#include "CODEC.H"

int codec_name_equal(const char *a, const char *b)
{
    unsigned char ca, cb;
    if (a == 0 || b == 0) return a == b;
    while (*a && *b) {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if (ca >= 'a' && ca <= 'z') ca = (unsigned char)(ca - 'a' + 'A');
        if (cb >= 'a' && cb <= 'z') cb = (unsigned char)(cb - 'a' + 'A');
        if (ca != cb) return 0;
    }
    return *a == *b;
}

static const Decoder decoders[] = {
    {"UTF-8", codec_decode_utf8},
    {"UTF-8-BOM", codec_decode_utf8},
    {"UTF-16LE", codec_decode_utf16le},
    {"UTF-16LE-BOM", codec_decode_utf16le},
    {"UTF-16BE", codec_decode_utf16be},
    {"UTF-16BE-BOM", codec_decode_utf16be},
    {"CP437", codec_decode_cp437},
    {"CP1252", codec_decode_cp1252},
    {"CP949", codec_decode_cp949},
    {0, 0}
};

static const Encoder encoders[] = {
    {"UTF-8", codec_encode_utf8},
    {"UTF-8-BOM", codec_encode_utf8},
    {"UTF-16LE", codec_encode_utf16le},
    {"UTF-16LE-BOM", codec_encode_utf16le},
    {"UTF-16BE", codec_encode_utf16be},
    {"UTF-16BE-BOM", codec_encode_utf16be},
    {"CP437", codec_encode_cp437},
    {"CP1252", codec_encode_cp1252},
    {"CP949", codec_encode_cp949},
    {0, 0}
};

const Decoder *codec_find_decoder(const char *name)
{
    int i;
    for (i = 0; decoders[i].name != 0; ++i)
        if (codec_name_equal(name, decoders[i].name)) return &decoders[i];
    return 0;
}

const Encoder *codec_find_encoder(const char *name)
{
    int i;
    for (i = 0; encoders[i].name != 0; ++i)
        if (codec_name_equal(name, encoders[i].name)) return &encoders[i];
    return 0;
}
