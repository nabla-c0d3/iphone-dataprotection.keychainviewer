//
//  RootViewController.h
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "keychain.h"

@interface RootViewController : UITableViewController {
	NSArray* keychainCategories;
	Keychain* keychain;
	NSDictionary* about;
}
@property (retain) NSArray* keychainCategories;
@property (retain) NSDictionary* about;
@property () Keychain* keychain;
@end
