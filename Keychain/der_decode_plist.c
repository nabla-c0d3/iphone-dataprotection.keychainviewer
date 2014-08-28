/**
    der_decode_plist implementation ripped from :

http://opensource.apple.com/source/xnu/xnu-2050.48.11/EXTERNAL_HEADERS/corecrypto/ccder.h
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_array.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_boolean.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_data.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_date.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_dictionary.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_null.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_plist.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/der_string.c
http://opensource.apple.com/source/Security/Security-55471/utilities/src/SecCFRelease.h

    including Apple license header just in case...
**/
/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <dlfcn.h>
#include "der_decode_plist.h"

#define CCDER_MULTIBYTE_TAGS  1

#ifdef CCDER_MULTIBYTE_TAGS
typedef unsigned long ccder_tag;
#else
typedef uint8_t ccder_tag;
#endif

enum
{
    CCDER_BOOLEAN=1,
    CCDER_INTEGER=0x2,
    CCDER_OCTET_STRING=4,
    CCDER_NULL=5,
    CCDER_UTF8_STRING=0xC,
    CCDER_GENERALIZED_TIME=0x18,

    CCDER_CONSTRUCTED_SEQUENCE=0x20000010,
    CCDER_CONSTRUCTED_SET=0x20000011,
};

typedef size_t cc_size;
typedef int ccoid_t;
typedef void cc_unit;

/*static const CFIndex kSecDERErrorUnknownEncoding = -1;
static const CFIndex kSecDERErrorUnsupportedCFObject = -2;
static const CFIndex kSecDERErrorUnsupportedDERType = -2;
static const CFIndex kSecDERErrorAllocationFailure = -3;
static const CFIndex kSecDERErrorUnsupportedNumberType = -4;
static const CFIndex kSecDERErrorUnderlyingError = -100;*/

const uint8_t* (*ccder_decode_tag)(ccder_tag *tagp, const uint8_t *der, const uint8_t *der_end) = NULL;
const uint8_t* (*ccder_decode_tl)(ccder_tag expected_tag, size_t *lenp, const uint8_t *der, const uint8_t *der_end) = NULL;
const uint8_t* (*ccder_decode_constructed_tl)(ccder_tag expected_tag, const uint8_t **body_end, const uint8_t *der, const uint8_t *der_end) = NULL;
const uint8_t* (*ccder_decode_sequence_tl)(const uint8_t **body_end, const uint8_t *der, const uint8_t *der_end) = NULL;

void __attribute__((constructor)) ccder_resolve()
{
    void* d = dlopen("/usr/lib/system/libcommonCrypto.dylib", RTLD_NOW);

    if (d != NULL)
    {
        ccder_decode_tag = dlsym(d, "ccder_decode_tag");
        ccder_decode_tl = dlsym(d, "ccder_decode_tl");
        ccder_decode_constructed_tl = dlsym(d, "ccder_decode_constructed_tl");
        ccder_decode_sequence_tl = dlsym(d, "ccder_decode_sequence_tl");
    }
}

#define CFReleaseNull(CF) { \
    CFTypeRef _cf = (CF);   \
    if (_cf) {              \
        (CF) = NULL;        \
        CFRelease(_cf);     \
        }                   \
    }

#define SecCFDERCreateError(a,b,c,d)


#define NULL_TIME NAN

CFAbsoluteTime SecCFGregorianDateGetAbsoluteTime(CFGregorianDate g, CFTimeInterval timeZoneOffset, CFErrorRef *error);
CFGregorianDate SecCFAbsoluteTimeGetGregorianDate(CFAbsoluteTime at, CFTimeInterval timeZoneOffset, CFErrorRef *error);

/* Cumalitive number of days in the year for months up to month i.  */
static int mdays[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

CFAbsoluteTime SecCFGregorianDateGetAbsoluteTime(CFGregorianDate g, CFTimeInterval timeZoneOffset, CFErrorRef *error) {
    int day = g.day;
    int is_leap_year = g.year % 4 == 0 && (g.year % 100 != 0 || g.year % 400 == 0) ? 1 : 0;
    if (g.month < 1 || g.month > 12 || day < 1 || day > 31 || g.hour >= 24 || g.minute >= 60 || g.second >= 60.0
        || (g.month == 2 && day > mdays[g.month] - mdays[g.month - 1] + is_leap_year)
        || (g.month != 2 && day > mdays[g.month] - mdays[g.month - 1])) {
        /* Invalid date. */
        SecCFDERCreateError(-1000, CFSTR("Invalid date."), 0, error);
        return NULL_TIME;
    }

    int dy = g.year - 2001;
    if (dy < 0) {
        dy += 1;
        day -= 1;
    }

    int leap_days = dy / 4 - dy / 100 + dy / 400;
    day += ((g.year - 2001) * 365 + leap_days) + mdays[g.month - 1] - 1;
    if (g.month > 2)
        day += is_leap_year;

#if 0
    int64_t time = day;
    time *= 24;
    time += g.hour;
    time *= 60;
    time += g.minute;
    time *= 60;
    time += lrint(g.second);
    time -= lrint(timeZoneOffset);
    return time;
#else
    CFAbsoluteTime absTime = (CFAbsoluteTime)((day * 24 + g.hour) * 60 + g.minute) * 60 + g.second;
    return absTime - timeZoneOffset;
#endif
}

CFGregorianDate SecCFAbsoluteTimeGetGregorianDate(CFAbsoluteTime at, CFTimeInterval timeZoneOffset, CFErrorRef *error) {
    CFTimeZoneRef tz = CFTimeZoneCreateWithTimeIntervalFromGMT(0, timeZoneOffset);
    if (!tz) {
        SecCFDERCreateError(-1000, CFSTR("timezone creation failed."), 0, error);
        CFGregorianDate g = {};
        return g;
    } else {
        CFGregorianDate g = CFAbsoluteTimeGetGregorianDate(at, tz);
        CFRelease(tz);
        return g;
    }
}


static int der_get_char(const uint8_t **der_p, const uint8_t *der_end,
                         CFErrorRef *error) {
    const uint8_t *der = *der_p;
    if (!der) {
        /* Don't create a new error in this case. */
        return -1;
    }

    if (der >= der_end) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                            CFSTR("Unexpected end of datetime"), 0, error);
        *der_p = NULL;
        return -1;
    }

    int ch = *der++;
    *der_p = der;
    return ch;
}


static int der_decode_decimal(const uint8_t **der_p, const uint8_t *der_end,
                             CFErrorRef *error) {
    char ch = der_get_char(der_p, der_end, error);
    if (ch < '0' || ch > '9') {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                            CFSTR("Not a decimal digit"), 0, error);
        *der_p = NULL;
        return -1;
    }
    return ch - '0';
}

static int der_decode_decimal_pair(const uint8_t **der_p, const uint8_t *der_end,
                            CFErrorRef *error) {
    return (10 * der_decode_decimal(der_p, der_end, error))
        + der_decode_decimal(der_p, der_end, error);
}

static int der_peek_byte(const uint8_t *der, const uint8_t *der_end) {
    if (!der || der >= der_end)
        return -1;

    return *der;
}

static const uint8_t *der_decode_decimal_fraction(double *fraction, CFErrorRef *error,
                                                  const uint8_t* der, const uint8_t *der_end) {
    int ch = der_peek_byte(der, der_end);
    if (ch == -1) {
        der = NULL;
    } else if (ch == '.') {
        uint64_t divisor = 1;
        uint64_t value = 0;
        int last = -1;
        while (++der < der_end) {
            last = ch;
            ch = *der;
            if (ch < '0' || ch > '9') {
                break;
            }
            if (divisor < UINT64_MAX / 10) {
                divisor *= 10;
                value *= 10;
                value += (ch - '0');
            }
        }
        if (der >= der_end)
            der = NULL;
        else if (last == '0') {
            SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                                CFSTR("fraction ends in 0"), 0, error);
            der = NULL;
        } else if (last == '.') {
            SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                                CFSTR("fraction without digits"), 0, error);
            der = NULL;
        } else {
            *fraction = (double)value / divisor;
        }
    } else {
        *fraction = 0.0;
    }

    return der;
}

static const CFTimeInterval der_decode_timezone_offset(const uint8_t **der_p,
                                                       const uint8_t *der_end,
                                                       CFErrorRef *error) {
    CFTimeInterval timeZoneOffset;
    int ch = der_get_char(der_p, der_end, error);
    if (ch == 'Z') {
        /* Zulu time. */
        timeZoneOffset = 0.0;
    } else {
        /* ZONE INDICATOR */
        int multiplier = 0;
        if (ch == '-')
            multiplier = -60;
        else if (ch == '+')
            multiplier = +60;
        else {
            SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                                CFSTR("Invalid datetime character"), 0, error);
            timeZoneOffset = NULL_TIME;
        }

        timeZoneOffset = multiplier *
            (der_decode_decimal_pair(der_p, der_end, error)
             * 60 + der_decode_decimal_pair(der_p, der_end, error));
    }
    return timeZoneOffset;
}

static const uint8_t* der_decode_commontime_body(CFAbsoluteTime *at, CFErrorRef *error, SInt32 year,
                                                 const uint8_t* der, const uint8_t *der_end)
{
    CFGregorianDate g;
    g.year = year;
    g.month = der_decode_decimal_pair(&der, der_end, error);
    g.day = der_decode_decimal_pair(&der, der_end, error);
    g.hour = der_decode_decimal_pair(&der, der_end, error);
    g.minute = der_decode_decimal_pair(&der, der_end, error);
    g.second = der_decode_decimal_pair(&der, der_end, error);
    double fraction;
    der = der_decode_decimal_fraction(&fraction, error, der, der_end);

    CFTimeInterval timeZoneOffset = der_decode_timezone_offset(&der, der_end, error);

#if 0
    secdebug("dateparse",
             "date %.*s year: %04d%02d%02d%02d%02d%02d%+05g",
             length, bytes, g.year, g.month,
             g.day, g.hour, g.minute, g.second,
             timeZoneOffset / 60);
#endif

    if (der) {
        if (der != der_end) {
            SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                                CFSTR("trailing garbage at end of datetime"), 0, error);
            return NULL;
        }

        *at = SecCFGregorianDateGetAbsoluteTime(g, timeZoneOffset, error) + fraction;
        if (*at == NULL_TIME)
            return NULL;
    }

    return der;
}

const uint8_t* der_decode_generalizedtime_body(CFAbsoluteTime *at, CFErrorRef *error,
                                               const uint8_t* der, const uint8_t *der_end)
{
    SInt32 year = 100 * der_decode_decimal_pair(&der, der_end, error) + der_decode_decimal_pair(&der, der_end, error);
    return der_decode_commontime_body(at, error, year, der, der_end);
}

const uint8_t* der_decode_universaltime_body(CFAbsoluteTime *at, CFErrorRef *error,
                                             const uint8_t* der, const uint8_t *der_end)
{
    SInt32 year = der_decode_decimal_pair(&der, der_end, error);
    if (year < 50) {
        /* 0  <= year <  50 : assume century 21 */
        year += 2000;
    } else if (year < 70) {
        /* 50 <= year <  70 : illegal per PKIX */
        SecCFDERCreateError(kSecDERErrorUnknownEncoding,
                            CFSTR("Invalid universal time year between 50 and 70"), 0, error);
        der = NULL;
    } else {
        /* 70 <  year <= 99 : assume century 20 */
        year += 1900;
    }

    return der_decode_commontime_body(at, error, year, der, der_end);
}

const uint8_t* der_decode_date(CFAllocatorRef allocator, CFOptionFlags mutability,
                               CFDateRef* date, CFErrorRef *error,
                               const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    der = ccder_decode_constructed_tl(CCDER_GENERALIZED_TIME, &der_end, der, der_end);
    CFAbsoluteTime at;
    der = der_decode_generalizedtime_body(&at, error, der, der_end);
    if (der) {
        *date = CFDateCreate(allocator, at);
        if (NULL == *date) {
            SecCFDERCreateError(kSecDERErrorUnderlyingError, CFSTR("Failed to create date"), NULL, error);
            return NULL;
        }
    }
    return der;
}

const uint8_t* der_decode_null(CFAllocatorRef allocator, CFOptionFlags mutability,
                                  CFNullRef* nul, CFErrorRef *error,
                                  const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    size_t payload_size = 0;
    const uint8_t *payload = ccder_decode_tl(CCDER_NULL, &payload_size, der, der_end);

    if (NULL == payload || payload_size != 0) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown null encoding"), NULL, error);
        return NULL;
    }

    *nul = kCFNull;

    return payload + payload_size;
}

const uint8_t* der_decode_boolean(CFAllocatorRef allocator, CFOptionFlags mutability,
                                  CFBooleanRef* boolean, CFErrorRef *error,
                                  const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    size_t payload_size = 0;
    const uint8_t *payload = ccder_decode_tl(CCDER_BOOLEAN, &payload_size, der, der_end);

    if (NULL == payload || (der_end - payload) < payload_size || payload_size != 1) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown boolean encoding"), NULL, error);
        return NULL;
    }

    *boolean = *payload ? kCFBooleanTrue : kCFBooleanFalse;

    return payload + payload_size;
}

const uint8_t* der_decode_data(CFAllocatorRef allocator, CFOptionFlags mutability,
                               CFDataRef* data, CFErrorRef *error,
                               const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    size_t payload_size = 0;
    const uint8_t *payload = ccder_decode_tl(CCDER_OCTET_STRING, &payload_size, der, der_end);

    if (NULL == payload || (der_end - payload) < payload_size) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown data encoding"), NULL, error);
        return NULL;
    }

    *data = CFDataCreate(allocator, payload, payload_size);

    if (NULL == *data) {
        SecCFDERCreateError(kSecDERErrorUnderlyingError, CFSTR("Failed to create data"), NULL, error);
        return NULL;
    }

    return payload + payload_size;
}

static const uint8_t* der_decode_key_value(CFAllocatorRef allocator, CFOptionFlags mutability,
                                           CFPropertyListRef* key, CFPropertyListRef* value, CFErrorRef *error,
                                           const uint8_t* der, const uint8_t *der_end)
{
    const uint8_t *payload_end = 0;
    const uint8_t *payload = ccder_decode_constructed_tl(CCDER_CONSTRUCTED_SEQUENCE, &payload_end, der, der_end);

    if (NULL == payload) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown data encoding, expected CCDER_CONSTRUCTED_SEQUENCE"), NULL, error);
        return NULL;
    }

    CFTypeRef keyObject = NULL;
    CFTypeRef valueObject = NULL;


    payload = der_decode_plist(allocator, mutability, &keyObject, error, payload, payload_end);
    payload = der_decode_plist(allocator, mutability, &valueObject, error, payload, payload_end);

    if (payload != NULL) {
        *key = keyObject;
        *value = valueObject;
    } else {
        CFReleaseNull(keyObject);
        CFReleaseNull(valueObject);
    }
    return payload;
}

const uint8_t* der_decode_dictionary(CFAllocatorRef allocator, CFOptionFlags mutability,
                                     CFDictionaryRef* dictionary, CFErrorRef *error,
                                     const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    const uint8_t *payload_end = 0;
    const uint8_t *payload = ccder_decode_constructed_tl(CCDER_CONSTRUCTED_SET, &payload_end, der, der_end);

    if (NULL == payload) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown data encoding, expected CCDER_CONSTRUCTED_SET"), NULL, error);
        return NULL;
    }


    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    if (NULL == dict) {
        SecCFDERCreateError(kSecDERErrorUnderlyingError, CFSTR("Failed to create data"), NULL, error);
        payload = NULL;
        goto exit;
    }

    while (payload != NULL && payload < payload_end) {
        CFTypeRef key = NULL;
        CFTypeRef value = NULL;
        
        payload = der_decode_key_value(allocator, mutability, &key, &value, error, payload, payload_end);
        
        if (payload) {
            CFDictionaryAddValue(dict, key, value);
            CFReleaseNull(key);
            CFReleaseNull(value);
        }
    }


exit:
    if (payload == payload_end) {
        *dictionary = dict;
        dict = NULL;
    }

    CFReleaseNull(dict);

    return payload;
}


const uint8_t* der_decode_number(CFAllocatorRef allocator, CFOptionFlags mutability,
                                 CFNumberRef* number, CFErrorRef *error,
                                 const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    size_t payload_size = 0;
    const uint8_t *payload = ccder_decode_tl(CCDER_INTEGER, &payload_size, der, der_end);

    if (NULL == payload || (der_end - payload) < payload_size) {
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown number encoding"), NULL, error);
        return NULL;
    }
    if (payload_size > sizeof(long long)) {
        SecCFDERCreateError(kSecDERErrorUnsupportedNumberType, CFSTR("Number too large"), NULL, error);
        return NULL;

    }

    long long value = 0;

    if (payload_size > 0) {
        if ((*payload & 0x80) == 0x80)
            value = -1; // Negative integers fill with 1s so we end up negative.
        
        const uint8_t* const payload_end = payload + payload_size;
        
        for (const uint8_t *payload_byte = payload;
             payload_byte < payload_end;
             ++payload_byte) {
            value <<= 8;
            value |= *payload_byte;
        }
    }

    *number = CFNumberCreate(allocator, kCFNumberLongLongType, &value);

    if (*number == NULL) {
        SecCFDERCreateError(kSecDERErrorAllocationFailure, CFSTR("Number allocation failed"), NULL, error);
        return NULL;
    }
    
    return payload + payload_size;
}

const uint8_t* der_decode_string(CFAllocatorRef allocator, CFOptionFlags mutability,
                                 CFStringRef* string, CFErrorRef *error,
                                 const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    size_t payload_size = 0;
    const uint8_t *payload = ccder_decode_tl(CCDER_UTF8_STRING, &payload_size, der, der_end);

    if (NULL == payload || (der_end - payload) < payload_size){
        SecCFDERCreateError(kSecDERErrorUnknownEncoding, CFSTR("Unknown string encoding"), NULL, error);
        return NULL;
    }

    *string = CFStringCreateWithBytes(allocator, payload, payload_size, kCFStringEncodingUTF8, false);

    if (NULL == *string) {
        SecCFDERCreateError(kSecDERErrorAllocationFailure, CFSTR("String allocation failed"), NULL, error);
        return NULL;
    }

    return payload + payload_size;
}

const uint8_t* der_decode_array(CFAllocatorRef allocator, CFOptionFlags mutability,
                                CFArrayRef* array, CFErrorRef *error,
                                const uint8_t* der, const uint8_t *der_end)
{
    if (NULL == der)
        return NULL;

    CFMutableArrayRef result = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);

    const uint8_t *elements_end;
    const uint8_t *current_element = ccder_decode_sequence_tl(&elements_end, der, der_end);
    
    while (current_element != NULL && current_element < elements_end) {
        CFPropertyListRef element = NULL;
        current_element = der_decode_plist(allocator, mutability, &element, error, current_element, elements_end);
        if (current_element) {
            CFArrayAppendValue(result, element);
            CFReleaseNull(element);
        }
    }

    if (current_element) {
        *array = result;
        result = NULL;
    }

    CFReleaseNull(result);
    return current_element;
}

const uint8_t* der_decode_plist(CFAllocatorRef allocator, CFOptionFlags mutability,
                                CFPropertyListRef* pl, CFErrorRef *error,
                                const uint8_t* der, const uint8_t *der_end)
{    if (NULL == der)
    return NULL;
    if (ccder_decode_tag == NULL)
    {
        printf("ccder functions missing, check /usr/lib/system/libcommonCrypto.dylib\n");
        return NULL;
    }

    ccder_tag tag;
    if (NULL == ccder_decode_tag(&tag, der, der_end))
        return NULL;

    switch (tag) {
        case CCDER_NULL:
            return der_decode_null(allocator, mutability, (CFNullRef*)pl, error, der, der_end);
        case CCDER_BOOLEAN:
            return der_decode_boolean(allocator, mutability, (CFBooleanRef*)pl, error, der, der_end);
        case CCDER_OCTET_STRING:
            return der_decode_data(allocator, mutability, (CFDataRef*)pl, error, der, der_end);
        case CCDER_GENERALIZED_TIME:
            return der_decode_date(allocator, mutability, (CFDateRef*)pl, error, der, der_end);
        case CCDER_CONSTRUCTED_SEQUENCE:
            return der_decode_array(allocator, mutability, (CFArrayRef*)pl, error, der, der_end);
        case CCDER_UTF8_STRING:
            return der_decode_string(allocator, mutability, (CFStringRef*)pl, error, der, der_end);
        case CCDER_INTEGER:
            return der_decode_number(allocator, mutability, (CFNumberRef*)pl, error, der, der_end);
        case CCDER_CONSTRUCTED_SET:
            return der_decode_dictionary(allocator, mutability, (CFDictionaryRef*)pl, error, der, der_end);
        default:
            SecCFDERCreateError(kSecDERErrorUnsupportedDERType, CFSTR("Unsupported DER Type"), NULL, error);
            return NULL;
    }
}
