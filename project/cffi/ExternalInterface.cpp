#ifndef STATIC_LINK
#define IMPLEMENT_API
#endif

#include "extension-icu.h"
#include "unicode/ubidi.h"
#include "unicode/ucnv.h" // For utf8 <-> utf16 conversion
#include <string>
#include <vector>

#include "unicode/utypes.h"
#include <stdio.h>
#include "unicode/udata.h"

static bool initialized = false;

static void nme_icu_init()
{
	printf("nme_icu_init\n");
	
	return;
	
	if (!initialized)
	{
		FILE *fileptr;
		char *buffer;
		long filelen;

		fileptr = fopen("icudata/icudt67l.dat", "rb");  // Open the file in binary mode
		fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
		filelen = ftell(fileptr);             // Get the current byte offset in the file
		rewind(fileptr);                      // Jump back to the beginning of the file
		
		buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
		fread(buffer, filelen, 1, fileptr); // Read in the entire file
		fclose(fileptr); // Close the file
		
		UErrorCode  status = U_ZERO_ERROR;
		udata_setCommonData(buffer, &status);
		
		if (U_FAILURE(status))
		{
			printf("%s\n", u_errorName(status));
		}
		
		initialized = true;
	}
}
DEFINE_PRIM(nme_icu_init, 0);

static value get_error_run(int input_len, int ubidi_direction)
{
	value error_run = alloc_empty_object();
	alloc_field(error_run, val_id("start"), alloc_int(0));
	alloc_field(error_run, val_id("length"), alloc_int(input_len));
	alloc_field(error_run, val_id("direction"), alloc_int(ubidi_direction));
	
	value error_array = alloc_array(1);
	val_array_set_i(error_array, 0, error_run);
	
	return error_array;
}

static value nme_icu_get_visual_runs(value text, value direction)
{
	if (!initialized)
	{
		nme_icu_init();
	}
	
	int ubidi_direction = val_int(direction) ? UBIDI_DEFAULT_RTL : UBIDI_DEFAULT_LTR;
	
	const char *utf8_text = val_string(text);
    size_t input_len = strlen(utf8_text);
	
    UErrorCode error = U_ZERO_ERROR;
    UConverter *converter = ucnv_open("UTF-8", &error);

    if (U_FAILURE(error))
    {
		printf("error while running ucnv_open(): %s\n", u_errorName(error));
		return get_error_run(input_len, ubidi_direction);
    }
	
	// Convert from utf-8 to utf-16
    size_t length;
    UChar *utf16str = new UChar[input_len + 1]();
    length = ucnv_toUChars(converter, utf16str, input_len + 1, utf8_text, -1, &error);
    if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;
		
		printf("error while running ucnv_toUChars(): %s\n", u_errorName(error));
		return get_error_run(input_len, ubidi_direction);
    }
	
	UBiDi* bidi = ubidi_openSized(length + 1, 0, &error);
	if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;

        printf("error while running ubidi_openSized(): %s\n", u_errorName(error));
		return get_error_run(input_len, ubidi_direction);
    }
	
	ubidi_setPara(bidi, utf16str, length, ubidi_direction, nullptr, &error);
	if (U_FAILURE(error))
    {
        ucnv_close(converter);
        delete[] utf16str;
        ubidi_close(bidi);

        printf("error while running ubidi_setPara(): %s\n", u_errorName(error));
		return get_error_run(input_len, ubidi_direction);
    }
	
	int count = ubidi_countRuns(bidi, &error);
	
	value result_array = alloc_array(count);
	
	for (int i = 0; i < count; ++i) 
	{
		int run_start;
		int run_length;
		UBiDiDirection run_direction = ubidi_getVisualRun(bidi, i, &run_start, &run_length);
		
		value run_object = alloc_empty_object();
		alloc_field(run_object, val_id("start"), alloc_int(run_start));
		alloc_field(run_object, val_id("length"), alloc_int(run_length));
		alloc_field(run_object, val_id("direction"), alloc_int(run_direction));
		
		val_array_set_i(result_array, i, run_object);
	}
	
	ubidi_close(bidi);
	
	// Delete the no longer needed utf16 buffer
    delete[] utf16str;
	
	ucnv_close(converter);
	
	return result_array;
}
DEFINE_PRIM(nme_icu_get_visual_runs, 2);

static value nme_icu_log2vis(value text) 
{
	if (!initialized)
	{
		nme_icu_init();
	}
	
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
