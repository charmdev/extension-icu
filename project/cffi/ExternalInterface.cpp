#ifndef STATIC_LINK
#define IMPLEMENT_API
#endif

#include "extension-icu.h"
#include "unicode/ubidi.h"
#include "unicode/ucnv.h" // For utf8 <-> utf16 conversion
#include <string>
#include <vector>


static value nme_icu_log2vis(value text) {
    const char *utf8_text = val_string(text);
    size_t input_len = strlen(utf8_text);

    UErrorCode error = U_ZERO_ERROR;
    UConverter *converter = ucnv_open("UTF-8", &error);

    if (U_FAILURE(error))
    {
        return alloc_string(u_errorName(error));
    }

    // Convert from utf-8 to utf-16
    size_t length;
    UChar *utf16str = new UChar[input_len + 1]();
    length = ucnv_toUChars(converter, utf16str, input_len + 1, utf8_text, -1, &error);
    if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;

        return alloc_string(u_errorName(error));
    }

    UChar *utf16strReordered = new UChar[length + 1]();

    UBiDi* bidi = ubidi_openSized(length + 1, 0, &error);
    if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;
        delete[] utf16strReordered;

        return alloc_string(u_errorName(error));
    }

    ubidi_setPara(bidi, utf16str, length, UBIDI_DEFAULT_RTL, nullptr, &error);
    if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;
        delete[] utf16strReordered;
        ubidi_close(bidi);

        return alloc_string(u_errorName(error));
    }

    length = ubidi_writeReordered(bidi, utf16strReordered, length + 1, UBIDI_DO_MIRRORING , &error);
    if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;
        delete[] utf16strReordered;
        ubidi_close(bidi);

        return alloc_string(u_errorName(error));
    }

    ubidi_close(bidi);

    // Convert back from utf-16 to utf-8
    size_t output_len = UCNV_GET_MAX_BYTES_FOR_STRING(length, ucnv_getMaxCharSize(converter));
    std::vector<char> utf8str(output_len);
    length = ucnv_fromUChars(converter, utf8str.data(), utf8str.size(), utf16strReordered, length, &error);
    if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;
        delete[] utf16strReordered;

        return alloc_string(u_errorName(error));
    }

    utf8str.resize(length);
    utf8str[length] = '\0';

    // Delete the no longer needed utf16 buffers
    delete[] utf16str;
    delete[] utf16strReordered;

    ucnv_close(converter);
    return alloc_string(utf8str.data());
}
DEFINE_PRIM(nme_icu_log2vis, 1);