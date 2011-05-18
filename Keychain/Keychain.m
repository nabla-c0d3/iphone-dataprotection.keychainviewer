//
//  Keychain.m
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Keychain.h"

void checkAccessibility(NSDictionary* res)
{
    NSData* data = [res objectForKey:@"data"];
    if (data != nil && [data length] >= 48) {
        const uint32_t* bytes = [data bytes];
        uint32_t clas = bytes[1];
        if (clas != kSecAttrAccessibleAlways && clas != kSecAttrAccessibleAlwaysThisDeviceOnly) {
            [res setValue:@"ok" forKey:@"safe_protection_class"];
        }
    }
    
}

@implementation Keychain
@synthesize db;

- (id) init {
	self.db = [FMDatabase databaseWithPath:@"/Library/Keychains/keychain-2.db"];
	if ([self.db openWithFlags:SQLITE_OPEN_READONLY] == NO)
    {
        NSString* testDbPath = [[NSBundle mainBundle] pathForResource:@"keychain-2" ofType:@"db"];

        self.db = [FMDatabase databaseWithPath:testDbPath];
        [self.db openWithFlags:SQLITE_OPEN_READONLY];
    }
    //TODO setuid back to mobile ?
	return self;
}

- (void) dealloc {
    [db release];
    [super dealloc];
}

- (NSArray*) getGenericPasswords {
	FMResultSet* rs = [self.db executeQuery:@"SELECT rowid, svce, acct, data, labl, agrp FROM genp ORDER BY svce,acct"];
	
	NSMutableArray* array = [[NSMutableArray alloc] init];
	while ([rs next]) {
		NSDictionary* dict = [rs resultDict];
        checkAccessibility(dict);
        
        [dict setValue:[rs stringForColumn:@"svce"] forKey:@"svce"];
        [dict setValue:[rs stringForColumn:@"labl"] forKey:@"labl"];
        [dict setValue:[rs stringForColumn:@"acct"] forKey:@"acct"];
        
		NSString* title = [dict objectForKey:@"acct"];
        NSString* svce = [dict objectForKey:@"svce"];
		if (![title length]) {
			title = [dict objectForKey:@"labl"];
		}
        if (![svce length]) {
            svce = [dict objectForKey:@"agrp"];
        }
		[dict setValue:title forKey:@"title"];
        [dict setValue:svce forKey:@"subtitle"];
		[array addObject: dict];
	}
	return array;
}

- (NSArray*) getInternetPasswords {
	FMResultSet* rs = [self.db executeQuery:@"SELECT rowid, srvr, port, ptcl, acct, data, labl, agrp FROM inet ORDER BY srvr,acct"];
	
	NSMutableArray* array = [[NSMutableArray alloc] init];
	while ([rs next]) {
		NSDictionary* dict = [rs resultDict];
        checkAccessibility(dict);

		[dict setValue:[dict objectForKey:@"acct"] forKey:@"title"];
		[dict setValue:[dict objectForKey:@"srvr"] forKey:@"subtitle"];
		[array addObject: dict];
	}
	return array;
}

- (NSArray*) getCertificates {
	FMResultSet* rs = [self.db executeQuery:@"SELECT rowid, subj, data, labl, agrp FROM cert ORDER BY labl"];
	
	NSMutableArray* array = [[NSMutableArray alloc] init];
	while ([rs next]) {
		NSDictionary* dict = [rs resultDict];
        checkAccessibility(dict);

        decrypt_cert_item(dict);
        if ([dict objectForKey:@"common_name"] ) {
            [dict setValue:[dict objectForKey:@"common_name"] forKey:@"title"];
            [dict setValue:[dict objectForKey:@"labl"] forKey:@"subtitle"];
        }
        else {
            [dict setValue:[dict objectForKey:@"labl"] forKey:@"title"];
		}
        [array addObject: dict];
	}
	return array;
}

- (NSArray*) getKeys {
    //hax, resultDict["data"] will map to the last one (keys.data)
	FMResultSet* rs = [self.db executeQuery:@"SELECT cert.data, keys.data, keys.rowid, keys.labl, klbl, cert.agrp FROM keys LEFT OUTER JOIN cert ON keys.klbl=cert.pkhh AND keys.agrp=cert.agrp ORDER BY keys.labl"];
	
	NSMutableArray* array = [[NSMutableArray alloc] init];
	while ([rs next]) {
		NSDictionary* dict = [rs resultDict];
        checkAccessibility(dict);

        [dict setValue:[dict objectForKey:@"labl"] forKey:@"title"];

        NSData* certdata = [rs dataForColumnIndex:0];
        if ([certdata isKindOfClass:[NSData class]]) {
            NSMutableDictionary* certdict = [NSMutableDictionary dictionaryWithObject:certdata forKey:@"data"];
            decrypt_cert_item(certdict);
            if ([certdict objectForKey:@"common_name"] ) {
                [dict setValue:[certdict objectForKey:@"common_name"] forKey:@"title"];
                [dict setValue:[certdict objectForKey:@"common_name"] forKey:@"common_name"];
                [dict setValue:[dict objectForKey:@"labl"] forKey:@"subtitle"];
            }
        }
		[array addObject: dict];
	}
	return array;
}
@end
