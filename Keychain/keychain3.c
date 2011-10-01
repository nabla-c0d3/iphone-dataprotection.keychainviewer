#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <unistd.h>
#include "IOKit.h"
#include "keychain.h"

CFDataRef decrypt_data_ios3(const uint8_t* datab, uint32_t len, uint32_t* pclass)
{
    return NULL;
}

CFMutableDictionaryRef keychain_get_item_ios3(sqlite3_stmt* stmt)
{
    return keychain_get_item(stmt, decrypt_data_ios3);
}
