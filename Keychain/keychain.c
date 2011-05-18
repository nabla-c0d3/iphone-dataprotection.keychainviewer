#include <CoreFoundation/CoreFoundation.h>
#include <CommonCrypto/CommonCryptor.h>
#include <Security/Security.h>
#include "IOKit.h"
#include "SecCert.h"

#define kAppleKeyStoreKeyUnwrap 11

CFStringRef protectionClassIdToString(uint32_t protection_class)
{
    static CFStringRef protectionClasses[] = {
        CFSTR("kSecAttrAccessibleWhenUnlocked"),
        CFSTR("kSecAttrAccessibleAfterFirstUnlock"),
        CFSTR("kSecAttrAccessibleAlways"),
        CFSTR("kSecAttrAccessibleWhenUnlockedThisDeviceOnly"),
        CFSTR("kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly"),
        CFSTR("kSecAttrAccessibleAlwaysThisDeviceOnly")
    };
    
    if (protection_class >= 6 && protection_class <= 11)
        return protectionClasses[protection_class - 6];
    return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Unknown protection class %d"), protection_class);
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

CFMutableDataRef decrypt_item(CFMutableDictionaryRef dict)
{
    uint8_t aes_key[48];
    IOReturn ret;
    size_t dataOutMoved = 0;
    
	CFDataRef data = CFDictionaryGetValue(dict, CFSTR("data"));
	
	if (data == NULL) {
		return NULL;
	}
	
    const uint8_t* datab = CFDataGetBytePtr(data);
    CFIndex len = CFDataGetLength(data);
    
    if (len < 48)
    {
        fprintf(stderr, "keychain item len < 48\n");
        return NULL;
    }
    
    //first by of second le dword
    uint32_t protection_class = datab[4];
    
	CFDictionarySetValue(dict, CFSTR("protection_class"), protectionClassIdToString(protection_class));
    
	uint32_t item_length = len-48;
    
    if((ret = AppleKeyStore_keyUnwrap(protection_class, &datab[8], 40, aes_key)))
	{
        fprintf(stderr, "AppleKeyStore_keyUnwrap = %x\n", ret);
		return NULL;
	}
    
    CFMutableDataRef item = CFDataCreateMutable(kCFAllocatorDefault, item_length);
    if (item == NULL)
    {
        memset(aes_key, 0, 48);
        return NULL;
    }
    CFDataSetLength(item, item_length);
    
    CCCryptorStatus cs = CCCrypt(kCCDecrypt,
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
        fprintf(stderr, "Keychain item decryption failed, CCCryptorStatus = %x\n", cs);
        CFRelease(item);
        return NULL;
    }
    CFDataSetLength(item, dataOutMoved);
    
    return item;    
}

void decrypt_password_item(CFMutableDictionaryRef dict)
{
    CFTypeRef item_value = NULL;

	CFMutableDataRef item_data = decrypt_item(dict);
    
	if (item_data != NULL) {
		item_value = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, item_data, kCFStringEncodingUTF8);

		if (item_value == NULL && !memcmp("bplist", CFDataGetBytePtr(item_data), 6))
            item_value = CFPropertyListCreateFromXMLData(NULL, item_data, 0, NULL);
        if (item_value == NULL)
            item_value = item_data;
    }
    else
    {
        item_value = CFSTR("Error! decryption failed");
    }
    CFDictionarySetValue(dict, CFSTR("password"), item_value);
    CFRelease(item_value);
    
    if (item_data != NULL && item_data != item_value)
        CFRelease(item_data);
}

void decrypt_cert_item(CFMutableDictionaryRef dict)
{
    CFDateRef not_valid_before, not_valid_after;
    CFStringRef subject_summary, issuer_summary;
    CFDataRef serial_number, pk_sha1;
    
	CFMutableDataRef item_data = decrypt_item(dict);
	if (item_data == NULL)
		return;
    
	SecCertificateRef cert = SecCertificateCreateWithData(NULL, item_data); 
	
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
    CFRelease(item_data);
	//CFShow(SecCertificateCopySummaryProperties(cert));
}

void decrypt_key_item(CFMutableDictionaryRef dict)
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
    
    CFMutableDataRef item_data = decrypt_item(dict);
	if (item_data == NULL)
		return;
        
    SecAsn1CoderRef coder = NULL;
    SECItem** dest = NULL;
    CSSM_DATA der_key = {CFDataGetLength(item_data), CFDataGetBytePtr(item_data)};
    
    SecAsn1CoderCreate(&coder);
    
    if (coder == NULL)
        return;

    if(SecAsn1DecodeData(coder, &der_key , kSecAsn1SequenceOfIntegerTemplate, &dest))
        return;

    int i = 0;
    while(i < 9 && *dest != NULL)
    {
        CFDataRef d = CFDataCreate(kCFAllocatorDefault, (*dest)->Data, (*dest)->Length);
        CFDictionarySetValue(dict, RSAPrivateKey_fields[i], d);
        dest++;
        i++;
    }
    //this seems to free all the stuff allocated by SecAsn1DecodeData
    SecAsn1CoderRelease(coder);
    CFRelease(item_data);
    return;
}

void keychain_process(CFMutableDictionaryRef dict)
{
    //check if already decrypted
	if (!CFDictionaryContainsKey(dict, CFSTR("protection_class")))
	{
		if (CFDictionaryContainsKey(dict, CFSTR("subj"))) {
			decrypt_cert_item(dict);
		}
		else if (CFDictionaryContainsKey(dict, CFSTR("klbl"))) {
			decrypt_key_item(dict);
		}
		else
		{
			decrypt_password_item(dict);
		}
	}
}
