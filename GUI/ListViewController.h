//
//  RootViewController.h
//  KeychainViewer
//
//  Created by admin on 4/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ListViewController : UITableViewController {
	NSArray* items;
	NSString* detailViewName;
}

@property (retain) NSArray* items;
@property (retain) NSString* detailViewName;
@end
