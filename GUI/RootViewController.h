//
//  RootViewController.h
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Keychain.h"

@interface RootViewController : UITableViewController {
	NSArray* keychainCategories;
	Keychain* keychain;
}
@property (retain) NSArray* keychainCategories;
@property (retain) Keychain* keychain;
@end
