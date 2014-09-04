#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <Security/Security.h>
#include <sqlite3.h>
#include "IOKit.h"
#include "SecCert.h"
#include "keychain.h"

#define kAppleKeyStoreInitUserClient 0
#define kAppleKeyStoreKeyUnwrap 11

CFStringRef keychain_protectionClassIdToString(uint32_t protection_class)
{
    static CFStringRef protectionClasses[] = {
        CFSTR("WhenUnlocked"),
        CFSTR("AfterFirstUnlock"),
        CFSTR("Always"),
        CFSTR("WhenUnlockedThisDeviceOnly"),
        CFSTR("AfterFirstUnlockThisDeviceOnly"),
        CFSTR("AlwaysThisDeviceOnly")
    };
    protection_class &= 0xF;

    if (protection_class >= 6 && protection_class <= 11)
        return protectionClasses[protection_class - 6];
    return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Unknown protection class %d"), protection_class);
}

int AppleKeyStoreKeyBagInit()
{
    uint64_t out = 0;
    uint32_t one = 1;
    return IOKit_call("AppleKeyStore",
                      kAppleKeyStoreInitUserClient,
                      NULL,
                      0,
                      NULL,
                      0,
                      &out,
                      &one,
                      NULL,
                      NULL);
}

IOReturn AppleKeyStore_keyUnwrap(uint32_t protection_class, const uint8_t* buffer, size_t bufferLen, uint8_t* out)
{
    size_t outputStructCnt = bufferLen+8;
    uint64_t input[2]={0, protection_class};
    
    return IOKit_call("AppleKeyStore",
                    kAppleKeyStoreKeyUnwrap,
                    input,
                    2,
                    buffer,
                    bufferLen,
                    NULL,
                    NULL,
                    out,
                    &outputStructCnt);
}

Keychain* keychain_open(const char* path)
{
    sqlite3_stmt *stmt;
    Keychain* k = malloc(sizeof(Keychain));
    if (k == NULL)
        return NULL;
    
    if (path == NULL)
        path = "/var/Keychains/keychain-2.db";
    
    if (sqlite3_open_v2(path, &k->db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "open_keychain : sqlite3_open_v2(\"%s\") fail\n", path);
        free(k);
        return NULL;
    }
    sqlite3_prepare_v2(k->db, "SELECT version FROM tversion", -1, &stmt, NULL);
    if(sqlite3_step(stmt) != SQLITE_ROW)
    {
        fprintf(stderr, "open_keychain : cannot get version from tversion table\n");
        free(k);
        return NULL;
    }
    k->version = sqlite3_column_int(stmt, 0);
    
    if (k->version  <= 3)
    {
        k->get_item = keychain_get_item_ios3;
    }
    else if (k->version == 4)
    {
        k->get_item = keychain_get_item_ios4;
    }
    else if (k->version >= 5)
    {
        k->get_item = keychain_get_item_ios5;
    }
    
    return k;
}

int keychain_close(Keychain* k)
{
    return sqlite3_close(k->db);
}


CFArrayRef keychain_get_items(Keychain* k, const char* sql, void (*callback)(Keychain*, CFMutableDictionaryRef))
{
    sqlite3_stmt *stmt;

    CFMutableArrayRef res = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    
    sqlite3_prepare_v2(k->db, sql, -1, &stmt, NULL);
    
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        CFMutableDictionaryRef item = k->get_item(stmt);
        if(item != NULL)
        {
            if (callback != NULL) {
                callback(k, item);
            }

            CFArrayAppendValue(res, item);
            CFRelease(item);
        }
    }
    sqlite3_finalize(stmt);
    return res;
}

CFMutableDictionaryRef keychain_get_item(sqlite3_stmt* stmt, CFDataRef (*decryptor)(const uint8_t*, uint32_t, uint32_t*))
{
    const char* name;
    const unsigned char* text;
    const void* blob;
    sqlite3_int64 num64;
    double dbl;
    CFNumberRef cfnum;
    CFDataRef cfd;
    CFStringRef cfs;
    uint32_t i, len, pclass;
    
    CFMutableDictionaryRef res = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    for(i=0; i < sqlite3_column_count(stmt) ; i++)
    {
        name = sqlite3_column_name(stmt,i);
        if(!strcmp(name, "data"))
        {
            blob = sqlite3_column_blob(stmt, i);
            len = sqlite3_column_bytes(stmt, i);
            CFDataRef data = decryptor(blob,len, &pclass);
            if( data != NULL)
            {
                CFDictionarySetValue(res, CFSTR("protection_class"), keychain_protectionClassIdToString(pclass));
                CFTypeRef data2 = keychain_convert_data_to_string_or_plist(data);
                CFDictionaryAddValue(res, CFSTR("data"), data2);
                CFRelease(data);

                if (data2 != data)
                    CFRelease(data2);
            }
        }
        else
        {
            CFStringRef cfname = CFStringCreateWithBytes(kCFAllocatorDefault, (const unsigned char*) name, strlen(name), kCFStringEncodingUTF8, false);
            //SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL
            switch(sqlite3_column_type(stmt, i))
            {
                case SQLITE_TEXT:
                    text = sqlite3_column_text(stmt, i);
                    cfs = CFStringCreateWithBytes(kCFAllocatorDefault, text, strlen((const char*)text), kCFStringEncodingUTF8, false);
                    if (cfs != NULL)
                    {
                        CFDictionaryAddValue(res, cfname, cfs);
                        CFRelease(cfs);
                    }
                    break;
                case SQLITE_BLOB:
                    blob = sqlite3_column_blob(stmt, i);
                    len = sqlite3_column_bytes(stmt, i);
                    cfd = CFDataCreate(kCFAllocatorDefault, blob, len);
                    if (cfd  != NULL)
                    {
                        CFDictionaryAddValue(res, cfname, cfd );
                        CFRelease(cfd );
                    }
                    break;
                case SQLITE_INTEGER:
                    num64 = sqlite3_column_int64(stmt, i);
                    cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &num64);
                    if (cfnum != NULL)
                    {
                        CFDictionaryAddValue(res, cfname, cfnum);
                        CFRelease(cfnum);
                    }
                    break;
                case SQLITE_FLOAT:
                    dbl = sqlite3_column_double(stmt, i);
                    cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &dbl);
                    if (cfnum != NULL)
                    {
                        CFDictionaryAddValue(res, cfname, cfnum);
                        CFRelease(cfnum);
                    }
                    break;
            }
            CFRelease(cfname);
        }
    }
    return res;
}

CFTypeRef keychain_convert_data_to_string_or_plist(CFDataRef data)
{
    CFTypeRef item_value = NULL;
    if( data == NULL || CFDataGetBytePtr(data) == NULL)
        return NULL;
    
    if (!memcmp("bplist", CFDataGetBytePtr(data), 6))
        item_value = CFPropertyListCreateFromXMLData(NULL, data, kCFPropertyListImmutable, NULL);
    
    if (item_value == NULL)
        item_value = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, data, kCFStringEncodingUTF8);
     
    if (item_value != NULL)
        return item_value;
    return data;
}

void keychain_parse_certificate(Keychain* k, CFMutableDictionaryRef dict)
{
    CFDateRef not_valid_before, not_valid_after;
    CFStringRef subject_summary, issuer_summary;
    CFDataRef serial_number, pk_sha1;
    
    if (CFDictionaryContainsKey(dict, CFSTR("common_name")))
        return;
    
    CFDataRef data = CFDictionaryGetValue(dict, CFSTR("data"));
    if (data == NULL || CFGetTypeID(data) != CFDataGetTypeID())
        return;
    
    SecCertificateRef cert = SecCertificateCreateWithData(NULL, data); 

    if (cert == NULL)
        return;
    
    subject_summary = SecCertificateCopySubjectSummary(cert);
    issuer_summary = SecCertificateCopyIssuerSummary(cert);
    not_valid_before = CFDateCreate(kCFAllocatorDefault, SecCertificateNotValidBefore(cert));
    not_valid_after = CFDateCreate(kCFAllocatorDefault, SecCertificateNotValidAfter(cert));
    serial_number = SecCertificateCopySerialNumber(cert);
    pk_sha1 = SecCertificateCopyPublicKeySHA1Digest(cert);
    
    if (subject_summary != NULL)
    {
        CFDictionarySetValue(dict, CFSTR("common_name"), subject_summary);
        CFRelease(subject_summary);
    }
    if (issuer_summary != NULL)
    {
        CFDictionarySetValue(dict, CFSTR("issuer"), issuer_summary);
        CFRelease(issuer_summary);
    }
    if (not_valid_before != NULL)
    {
        CFDictionarySetValue(dict, CFSTR("not_valid_before"), not_valid_before);
        CFRelease(not_valid_before);
    }
    if (not_valid_after != NULL)
    {
        CFDictionarySetValue(dict, CFSTR("not_valid_after"), not_valid_after);
        CFRelease(not_valid_after);
    }
    if (serial_number != NULL)
    {
        CFDictionarySetValue(dict, CFSTR("serial_number"), serial_number);
        CFRelease(serial_number);
    }
    if (pk_sha1 != NULL)
    {
        CFDictionarySetValue(dict, CFSTR("pk_sha1"), pk_sha1);
        CFRelease(pk_sha1);
    }
    CFRelease(cert);
}

void keychain_add_cn_for_key(Keychain* k, CFMutableDictionaryRef dict)
{
    sqlite3_stmt* stmt;
    
    CFDataRef klbl = CFDictionaryGetValue(dict, CFSTR("klbl"));
    
    if (klbl == NULL || CFGetTypeID(klbl) != CFDataGetTypeID())
        return;
    
    sqlite3_prepare_v2(k->db, "SELECT data FROM cert WHERE pkhh=?", -1, &stmt, NULL);
    
    sqlite3_bind_blob(stmt, 1, CFDataGetBytePtr(klbl), CFDataGetLength(klbl), SQLITE_STATIC);
    
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        CFMutableDictionaryRef cert = k->get_item(stmt);
        if (cert != NULL) {
            keychain_parse_certificate(k, cert);
            CFStringRef cn = CFDictionaryGetValue(cert, CFSTR("common_name"));
            if (cn != NULL) {
                CFDictionarySetValue(dict, CFSTR("common_name"), cn);
            }
            CFRelease(cert);
            break;
        }
    }
    sqlite3_finalize(stmt);
}

void keychain_parse_key(Keychain* k, CFMutableDictionaryRef dict)
{
/**
RSAPrivateKey ::= SEQUENCE {
   version                 Version,
   modulus                 INTEGER, -- (Usually large) n
   publicExponent          INTEGER, -- (Usually small) e
   privateExponent         INTEGER, -- (Usually large) d
   prime1                  INTEGER, -- (Usually large) p
   prime2                  INTEGER, -- (Usually large) q
   exponent1               INTEGER, -- (Usually large) d mod (p-1)
   exponent2               INTEGER, -- (Usually large) d mod (q-1)
   coefficient             INTEGER, -- (Usually large) (inverse of q) mod p
   otherPrimeInfos         OtherPrimeInfos OPTIONAL
 }
**/
    static CFStringRef RSAPrivateKey_fields[] = {CFSTR("version"),
        CFSTR("modulus"),
        CFSTR("public_exponent"),
        CFSTR("private_exponent"),
        CFSTR("prime1"),
        CFSTR("prime2"),
        CFSTR("exponent1"),
        CFSTR("exponent2"),
        CFSTR("coefficient"),
    };
    
    CFDataRef data = CFDictionaryGetValue(dict, CFSTR("data"));
    if (data == NULL || CFGetTypeID(data) != CFDataGetTypeID())
        return;
        
    SecAsn1CoderRef coder = NULL;
    SECItem** dest = NULL;
    CSSM_DATA der_key = {CFDataGetLength(data), (uint8_t*) CFDataGetBytePtr(data)};
    
    SecAsn1CoderCreate(&coder);
    
    if (coder == NULL)
        return;

    if(SecAsn1DecodeData(coder, &der_key , kSecAsn1SequenceOfIntegerTemplate, &dest))
        return;

    int i = 0;
    while(i < 9 && *dest != NULL)
    {
        CFDataRef d = CFDataCreate(kCFAllocatorDefault, (*dest)->Data, (*dest)->Length);
        if (d != NULL)
        {
            CFDictionarySetValue(dict, RSAPrivateKey_fields[i], d);
            CFRelease(d);
        }
        dest++;
        i++;
    }
    //this seems to free all the stuff allocated by SecAsn1DecodeData
    SecAsn1CoderRelease(coder);
    
    keychain_add_cn_for_key(k, dict);
    return;
}

CFArrayRef keychain_get_passwords(Keychain* k)
{
    return keychain_get_items(k, "SELECT * from genp ORDER BY svce,acct", NULL);
}

CFArrayRef keychain_get_internet_passwords(Keychain* k)
{
    return keychain_get_items(k, "SELECT * from inet ORDER BY srvr,acct", NULL);
}

CFArrayRef keychain_get_certs(Keychain* k)
{
    return keychain_get_items(k, "SELECT * from cert ORDER BY labl", keychain_parse_certificate);
}

CFArrayRef keychain_get_keys(Keychain* k)
{
    return keychain_get_items(k, "SELECT * from keys ORDER BY labl", keychain_parse_key);
}
