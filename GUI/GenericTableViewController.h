//
//  GenericTableViewController.h
//  KeychainViewer
//
//  Created by admin on 4/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface GenericTableViewController : UITableViewController {
	id object;
	int numRows;
	BOOL isDictionary;
	BOOL isArray;
	UITableViewCellStyle cellStyle;
}

- (id)initWithObject:(id)obj;

@end
