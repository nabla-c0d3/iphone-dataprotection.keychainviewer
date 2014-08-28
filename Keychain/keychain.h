#include <sqlite3.h>
#include "IOKit.h"

#define kSecAttrAccessibleWhenUnlocked                      6
#define kSecAttrAccessibleAfterFirstUnlock                  7
#define kSecAttrAccessibleAlways                            8
#define kSecAttrAccessibleWhenUnlockedThisDeviceOnly        9
#define kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly    10
#define kSecAttrAccessibleAlwaysThisDeviceOnly              11

typedef struct Keychain
{
    sqlite3 *db;
    uint32_t version;
    CFMutableDictionaryRef (*get_item)(sqlite3_stmt*);
} Keychain;

int AppleKeyStoreKeyBagInit();
IOReturn AppleKeyStore_keyUnwrap(uint32_t protection_class, const uint8_t* buffer, size_t bufferLen, uint8_t* out);

Keychain* keychain_open(const char* path);
int keychain_close(Keychain*);
CFArrayRef keychain_get_items(Keychain* k, const char* sql, void (*callback)(Keychain*, CFMutableDictionaryRef));
CFMutableDictionaryRef keychain_get_item(sqlite3_stmt* stmt, CFDataRef (*decryptor)(const uint8_t*, uint32_t, uint32_t*));

void keychain_parse_certificate(Keychain* k, CFMutableDictionaryRef dict);
void keychain_parse_key(Keychain* k, CFMutableDictionaryRef dict);

CFTypeRef keychain_convert_data_to_string_or_plist(CFDataRef data);

CFMutableDictionaryRef keychain_get_item_ios3(sqlite3_stmt* stmt);
CFMutableDictionaryRef keychain_get_item_ios4(sqlite3_stmt* stmt);
CFMutableDictionaryRef keychain_get_item_ios5(sqlite3_stmt* stmt);

CFArrayRef keychain_get_passwords(Keychain* k);
CFArrayRef keychain_get_internet_passwords(Keychain* k);
CFArrayRef keychain_get_certs(Keychain* k);
CFArrayRef keychain_get_keys(Keychain* k);

CFStringRef keychain_protectionClassIdToString(uint32_t protection_class);
