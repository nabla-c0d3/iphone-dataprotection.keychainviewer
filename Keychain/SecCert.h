
CFStringRef SecCertificateCopyIssuerSummary(SecCertificateRef);
CFStringRef SecCertificateCopySummaryProperties(SecCertificateRef);
CFDataRef SecCertificateCopySerialNumber(SecCertificateRef);
CFDataRef SecCertificateCopyPublicKeySHA1Digest(SecCertificateRef);
SecCertificateRef SecCertificateCreateWithData(CFAllocatorRef, CFDataRef); 
CFAbsoluteTime SecCertificateNotValidAfter(SecCertificateRef);
CFAbsoluteTime SecCertificateNotValidBefore(SecCertificateRef);

//ripped off from various headers

typedef struct SecAsn1Coder *SecAsn1CoderRef;



typedef size_t CSSM_SIZE;
	
typedef struct cssm_data {
    CSSM_SIZE Length; /* in bytes */
    uint8_t *Data;
} CSSM_DATA, *CSSM_DATA_PTR;
typedef CSSM_DATA SECItem;
typedef struct SecAsn1Template_struct {
    uint32_t kind;
    uint32_t offset;
    const void *sub;
    uint32_t size;
} SecAsn1Template;

#define SEC_ASN1_INTEGER			0x02
#define SEC_ASN1_SEQUENCE			0x10
#define SEC_ASN1_GROUP		0x02000	
#define SEC_ASN1_SIGNED_INT	0X800000
#define SEC_ASN1_SEQUENCE_OF	(SEC_ASN1_GROUP | SEC_ASN1_SEQUENCE)

static const SecAsn1Template kSecAsn1IntegerTemplate[] = {
    { SEC_ASN1_INTEGER | SEC_ASN1_SIGNED_INT, 0, NULL, sizeof(SECItem) }
};
static  const SecAsn1Template kSecAsn1SequenceOfIntegerTemplate[] = {
    { SEC_ASN1_SEQUENCE_OF, 0, kSecAsn1IntegerTemplate }
};

/*
 * Create/destroy SecAsn1Coder object. 
 */
OSStatus SecAsn1CoderCreate(SecAsn1CoderRef  *coder);
OSStatus SecAsn1CoderRelease(SecAsn1CoderRef  coder);
OSStatus SecAsn1DecodeData(SecAsn1CoderRef			coder,
                           const CSSM_DATA			*src,
                           const SecAsn1Template 	*templ,	
                           void					*dest);

/**
SecCertificateCopyCommonNames
SecCertificateCopyCompanyName
SecCertificateCopyDNSNames
SecCertificateCopyData
SecCertificateCopyExtendedKeyUsage
SecCertificateCopyIPAddresses
SecCertificateCopyIssuerSHA1Digest
SecCertificateCopyIssuerSequence
SecCertificateCopyIssuerSummary
SecCertificateCopyNTPrincipalNames
SecCertificateCopyOrganization
SecCertificateCopyProperties
SecCertificateCopyPublicKey
SecCertificateCopyPublicKeySHA1Digest
SecCertificateCopyRFC822Names
SecCertificateCopySerialNumber
SecCertificateCopySubjectSequence
SecCertificateCopySubjectString
SecCertificateCopySubjectSummary
SecCertificateCopySummaryProperties
SecCertificateCreate
SecCertificateCreateWithBytes
SecCertificateCreateWithData
SecCertificateCreateWithPEM
SecCertificateDataArrayCopyArray
SecCertificateGetAuthorityKeyID
SecCertificateGetBasicConstraints
SecCertificateGetBytePtr
SecCertificateGetCAIssuers
SecCertificateGetCertificatePolicies
SecCertificateGetInhibitAnyPolicySkipCerts
SecCertificateGetKeyUsage
SecCertificateGetLength
SecCertificateGetNormalizedIssuerContent
SecCertificateGetNormalizedSubjectContent
SecCertificateGetOCSPResponders
SecCertificateGetPolicyConstraints
SecCertificateGetPublicKeyAlgorithm
SecCertificateGetPublicKeyData
SecCertificateGetSHA1Digest
SecCertificateGetSubjectKeyID
SecCertificateGetTypeID
SecCertificateHasCriticalSubjectAltName
SecCertificateHasSubject
SecCertificateHasUnknownCriticalExtension
SecCertificateIsSelfSignedCA
SecCertificateIsValid
SecCertificateNotValidAfter
SecCertificateNotValidBefore
SecCertificatePathCopyAddingLeaf
SecCertificatePathCopyArray
SecCertificatePathCopyFromParent
SecCertificatePathCopyPublicKeyAtIndex
SecCertificatePathCreate
SecCertificatePathGetCertificateAtIndex
SecCertificatePathGetCount
SecCertificatePathGetIndexOfCertificate
SecCertificatePathGetNextSourceIndex
SecCertificatePathGetRoot
SecCertificatePathIsAnchored
SecCertificatePathScore
SecCertificatePathSelfSignedIndex
SecCertificatePathSetIsAnchored
SecCertificatePathSetNextSourceIndex
SecCertificatePathSetSelfIssued
SecCertificatePathVerify
SecCertificateVersion**/
