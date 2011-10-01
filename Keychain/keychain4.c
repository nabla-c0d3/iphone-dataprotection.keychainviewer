#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include "IOKit.h"
#include "keychain.h"

CFDataRef decrypt_data_ios4(const uint8_t* datab, uint32_t len, uint32_t* pclass)
{
    uint8_t aes_key[48];
    uint32_t version, protection_class, item_length;
    CCCryptorStatus cs;
    IOReturn ret;
    size_t dataOutMoved = 0;
    
    if (len < 48)
    {
        fprintf(stderr, "decrypt_data_ios4 : keychain item len < 48\n");
        return NULL;
    }
    version = ((uint32_t*) datab)[0];
    protection_class = ((uint32_t*) datab)[1];
    if (pclass != NULL)
        *pclass = protection_class;
    item_length = len-48;
    
    if (version != 0)
    {
        fprintf(stderr, "decrypt_data_ios4 : version != 0\n");
        return NULL;
    }
        
    if((ret = AppleKeyStore_keyUnwrap(protection_class, &datab[8], 40, aes_key)))
    {
        fprintf(stderr, "decrypt_data_ios4 : AppleKeyStore_keyUnwrap = %x\n", ret);
        return NULL;
    }
    
    CFMutableDataRef item = CFDataCreateMutable(kCFAllocatorDefault, item_length);
    if (item == NULL)
    {
        memset(aes_key, 0, 48);
        return NULL;
    }
    CFDataSetLength(item, item_length);
    
    cs = CCCrypt(kCCDecrypt,
                 kCCAlgorithmAES128,
                 kCCOptionPKCS7Padding,
                 aes_key,
                 32,
                 NULL,
                 &datab[48],
                 item_length,
                 (void*) CFDataGetBytePtr(item),
                 item_length,
                 &dataOutMoved);

    memset(aes_key, 0, 48);
    if (cs != 0)
    {
        fprintf(stderr, "decrypt_data_ios4 :  CCCrypt failed, CCCryptorStatus = %x\n", cs);
        CFRelease(item);
        return NULL;
    }
    //remove padding
    CFDataSetLength(item, dataOutMoved);
    
    return item;    
}

CFMutableDictionaryRef keychain_get_item_ios4(sqlite3_stmt* stmt)
{
    return keychain_get_item(stmt, decrypt_data_ios4);
}