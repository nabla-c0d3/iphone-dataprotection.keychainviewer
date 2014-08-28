#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <unistd.h>
#include "IOKit.h"
#include "keychain.h"

#define kIOAESAcceleratorTask 1

#define kIOAESAcceleratorEncrypt 0
#define kIOAESAcceleratorDecrypt 1

#define kIOAESAccelerator835Mask 0x835

typedef struct 
{
    void*       cleartext;
    void*       ciphertext;
    uint32_t    size;
    uint8_t     iv[16];
    uint32_t    mode;
    uint32_t    bits;
    uint8_t     keybuf[32];
    uint32_t    mask;
    uint32_t    zero; //only on iOS 4
} IOAESStruct;


CFDataRef decrypt_data_ios3(const uint8_t* datab, uint32_t len, uint32_t* pclass)
{
    IOAESStruct in;
    IOReturn ret;
    unsigned char md[20];
    unsigned char pad;
    size_t IOAESStructSize = sizeof(IOAESStruct) - 4; //iOS 3 size
    
    if (pclass != NULL)
        *pclass = kSecAttrAccessibleAlways;
    
    if (len < 48) //iv + sha1 (20, rounded to 32) = 16 + 32
    {
        fprintf(stderr, "decrypt_data_ios3 : len < 48\n");
        return NULL;
    }
    len = len - 16;

    uint8_t* buffer = valloc(len);
    if (buffer == NULL)
    {
        return NULL;
    }
    uint8_t* output_buffer = valloc(len);
    if (output_buffer == NULL)
    {
        free(buffer);
        return NULL;
    }
    memcpy(buffer, &datab[16], len);
    
    in.mode = kIOAESAcceleratorDecrypt;
    in.bits = 128;
    in.cleartext = output_buffer;
    in.ciphertext = buffer;
    in.size = len;
    in.mask = kIOAESAccelerator835Mask;

    memset(in.keybuf, 0, sizeof(in.keybuf));

    memcpy(in.iv, datab, 16);

    io_service_t conn = IOKit_getConnect("IOAESAccelerator");
    
    uid_t saved_uid = geteuid();
    if(seteuid(64) == -1) //securityd
    {
        fprintf(stderr, "seteuid(_securityd) failed, errno=%d\n", errno);
    }
    if(geteuid() != 64) //HAX otherwise looks like the new uid isnt "commited"
    {
        fprintf(stderr,"geteuid=%x\n", geteuid()); 
    }
    ret = IOConnectCallStructMethod(conn, kIOAESAcceleratorTask, &in, IOAESStructSize, &in, &IOAESStructSize);

    if (ret == 0xe00002c2) //if we have an iOS 3 keychain on iOS 4
    {
        IOAESStructSize += 4;
        ret = IOConnectCallStructMethod(conn, kIOAESAcceleratorTask, &in, IOAESStructSize, &in, &IOAESStructSize);
    }
    if (seteuid(saved_uid) == -1)
    {
        fprintf(stderr, "seteuid(saved_uid) failed, errno=%d\n", errno);
    }

    if (ret != 0)
    {
        fprintf(stderr, "decrypt_data_ios3 : saved_uid=%d IOConnectCallStructMethod = %x\n", saved_uid, ret);
        free(buffer);
        free(output_buffer);
        return NULL;
    }
    pad = output_buffer[len - 1];
    if (pad > 16 || pad > len || pad == 0) {
        fprintf(stderr, "decrypt_data_ios3 : bad padding = %x\n", pad);
        free(buffer);
        free(output_buffer);
        return NULL;
    }
    len = len - 20 - pad;
    if (len & 0x80000000) {
        fprintf(stderr, "decrypt_data_ios3 : length underflow, should not happen len= %x\n", len);
        free(buffer);
        free(output_buffer);
        return NULL;
    }
    CC_SHA1(output_buffer, len, md);
    if (memcmp(&output_buffer[len], md, 20)) {
        fprintf(stderr, "decrypt_data_ios3 : SHA1 mismatch\n");
        free(buffer);
        free(output_buffer);
        return NULL;
    }
    CFDataRef data = CFDataCreate(kCFAllocatorDefault, output_buffer, len);
    free(buffer);
    free(output_buffer);
    
    return data;
}

CFMutableDictionaryRef keychain_get_item_ios3(sqlite3_stmt* stmt)
{
    return keychain_get_item(stmt, decrypt_data_ios3);
}
