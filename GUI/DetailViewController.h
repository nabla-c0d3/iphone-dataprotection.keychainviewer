//
//  PasswordViewController.h
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface DetailViewController : UITableViewController {
	NSArray* description;
	NSDictionary* data;
}
@property (retain) NSArray* description;
@property (retain) NSDictionary* data;

- (id) initWithPlist:(NSString*) plist;
@end
