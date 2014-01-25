#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <Security/Security.h>
#include <dlfcn.h>
#include <sqlite3.h>
#include "IOKit.h"
#include "keychain.h"
#include "der_decode_plist.h"

//HAX to compile without ios 5 SDK
int (*CCCryptorGCM)() = NULL;

void getCCCryptorGCM()
{
    void* d = dlopen("/usr/lib/system/libcommonCrypto.dylib", RTLD_NOW);
    
    CCCryptorGCM = dlsym(d, "CCCryptorGCM");
    //fprintf(stderr, "CCCryptorGCM=%x\n", CCCryptorGCM);
}

CFMutableDictionaryRef decrypt_data_ios5(const uint8_t* datab, uint32_t len, uint32_t* pclass)
{
    CFMutableDictionaryRef plist = NULL;
    CFErrorRef err = NULL;
    uint8_t aes_key[48];
    uint32_t version, protection_class, wrapped_length, item_length;
    CCCryptorStatus cs = 0;
    IOReturn ret;
    int taglen = 16;
    char tag[16];
    
    if (len < 68)
    {
        fprintf(stderr, "decrypt_data_ios5 : keychain item len < 68\n");
        return NULL;
    }
    version = ((uint32_t*) datab)[0];
    protection_class = ((uint32_t*) datab)[1];
    if (pclass != NULL)
        *pclass = protection_class;
    wrapped_length = ((uint32_t*) datab)[2];
    item_length = len - 48 - 4 - 16;

    if (version != 2 && version != 3)
    {
        fprintf(stderr, "decrypt_data_ios5 : version = %d\n", version);
        return NULL;
    }
    if (wrapped_length != 40)
    {
        fprintf(stderr, "decrypt_data_ios5 : wrapped_length != 0x28\n");
        return NULL;
    }
    
    if((ret = AppleKeyStore_keyUnwrap(protection_class, &datab[12], 40, aes_key)))
    {
        fprintf(stderr, "decrypt_data_ios5 : AppleKeyStore_keyUnwrap = %x\n", ret);
        return NULL;
    }
    
    CFMutableDataRef item = CFDataCreateMutable(kCFAllocatorDefault, item_length);
    if (item == NULL)
    {
        memset(aes_key, 0, 48);
        return NULL;
    }
    CFDataSetLength(item, item_length);
        
    if (CCCryptorGCM == NULL)
        getCCCryptorGCM();
    if (CCCryptorGCM != NULL)
        cs = CCCryptorGCM(kCCDecrypt,
                     kCCAlgorithmAES128,
                     aes_key,
                     32,
                     0,
                     0,
                     0,
                     0,
                     &datab[52],
                     item_length,
                     (void*) CFDataGetBytePtr(item),
                     tag,
                     &taglen);

    memset(aes_key, 0, 48);
    if (cs != 0)
    {
        fprintf(stderr, "decrypt_data_ios5 : CCCryptorGCM failed, CCCryptorStatus = %x\n", cs);
        CFRelease(item);
        return NULL;
    }
    
    if (version == 3)
    {
        der_decode_plist(kCFAllocatorDefault, 1,
                         (CFPropertyListRef*) &plist, &err,
                         (const uint8_t*) CFDataGetBytePtr(item),
                         (const uint8_t*) CFDataGetBytePtr(item) + item_length);
    }
    else
    {
        plist = (CFMutableDictionaryRef) CFPropertyListCreateFromXMLData(NULL, item, kCFPropertyListMutableContainersAndLeaves, NULL);
    }
    CFRelease(item);

    if (plist != NULL && CFGetTypeID(plist) != CFDictionaryGetTypeID())
    {
        fprintf(stderr, "decrypt_data_ios5 : CFPropertyListCreateFromXMLData did not return a dictionary\n");
        CFRelease(plist);
        return NULL;
    }
    return plist;
}

CFMutableDictionaryRef keychain_get_item_ios5(sqlite3_stmt* stmt)
{
    const char* name;
    uint32_t pclass, i, rowid;

    for(i=0; i < sqlite3_column_count(stmt) ; i++)
    {
        name = sqlite3_column_name(stmt,i);
        if(!strcmp(name, "rowid"))
        {
            rowid = sqlite3_column_int(stmt, i);
        }
        if(!strcmp(name, "data"))
        {
            const void* blob = sqlite3_column_blob(stmt, i);
            int len = sqlite3_column_bytes(stmt, i);
            CFMutableDictionaryRef item = decrypt_data_ios5(blob,len,&pclass);
            if (item != NULL)
            {
                CFNumberRef cf_rowid = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &rowid);
                CFDictionarySetValue(item, CFSTR("rowid"), cf_rowid);
                CFRelease(cf_rowid);
                CFDictionarySetValue(item, CFSTR("protection_class"), keychain_protectionClassIdToString(pclass));
                CFDataRef vdata = CFDictionaryGetValue(item, CFSTR("v_Data"));
                CFTypeRef data = keychain_convert_data_to_string_or_plist(vdata);
                if(data != NULL)
                {
                    CFDictionaryAddValue(item, CFSTR("data"), data);
                    if (data != vdata)
                        CFRelease(data);
                }
                return item;
            }
        }
    }
    return NULL;
}
