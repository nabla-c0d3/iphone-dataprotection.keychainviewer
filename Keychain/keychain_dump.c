#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include "IOKit.h"
#include "keychain.h"

void saveResults(CFStringRef filename, CFTypeRef out)
{
    CFURLRef fileURL = CFURLCreateWithFileSystemPath( NULL, filename, kCFURLPOSIXPathStyle, FALSE);
    CFWriteStreamRef stream = CFWriteStreamCreateWithFile( NULL, fileURL);
    CFWriteStreamOpen(stream);
    CFPropertyListWriteToStream(out, stream, kCFPropertyListXMLFormat_v1_0, NULL);
    CFWriteStreamClose(stream);
    
    CFRelease(stream);
    CFRelease(fileURL);
}

int main(int argc, char* argv[])
{
    AppleKeyStoreKeyBagInit();
    Keychain* k = keychain_open(argc > 1 ? argv[1] : NULL);
    if (k == NULL)
        return -1;
    
    CFArrayRef p = keychain_get_items(k, "SELECT * from genp", NULL);
    printf("Writing %d passwords to genp.plist\n", (int) CFArrayGetCount(p));
    saveResults(CFSTR("genp.plist"), p);
    
    p = keychain_get_items(k, "SELECT * from inet", NULL);
    saveResults(CFSTR("inet.plist"), p);
    printf("Writing %d internet passwords to inet.plist\n", (int) CFArrayGetCount(p));
    
    p = keychain_get_items(k, "SELECT * from cert", NULL);
    saveResults(CFSTR("cert.plist"), p);
    printf("Writing %d certificates to cert.plist\n", (int) CFArrayGetCount(p));
    
    p = keychain_get_items(k, "SELECT * from keys", NULL);
    saveResults(CFSTR("keys.plist"), p);
    printf("Writing %d keys to keys.plist\n", (int) CFArrayGetCount(p));
    
    keychain_close(k);
    return 0;
}

