//
//  Keychain.h
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "FMDatabase.h"

#define kSecAttrAccessibleWhenUnlocked                      6
#define kSecAttrAccessibleAfterFirstUnlock                  7
#define kSecAttrAccessibleAlways                            8
#define kSecAttrAccessibleWhenUnlockedThisDeviceOnly        9
#define kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly    10
#define kSecAttrAccessibleAlwaysThisDeviceOnly              11

@interface Keychain : NSObject {
	FMDatabase* db;
}
@property (retain) FMDatabase* db;

- (NSArray*) getGenericPasswords;
- (NSArray*) getInternetPasswords;
- (NSArray*) getCertificates;
- (NSArray*) getKeys;
@end
