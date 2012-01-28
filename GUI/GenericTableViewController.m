//
//  GenericTableViewController.m
//  KeychainViewer
//
//  Created by admin on 4/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GenericTableViewController.h"


@implementation GenericTableViewController

#pragma mark -
#pragma mark Initialization


- (id)initWithObject:(id)obj {
    self = [super initWithStyle:UITableViewStylePlain];
    if (self) {
        cellStyle = UITableViewCellStyleDefault;
        numRows = 0;
        isDictionary = [obj isKindOfClass:[NSDictionary class]];
        isArray = [obj isKindOfClass:[NSArray class]];
        if (isDictionary || isArray)
        {
            [obj retain];
            object = obj;
            numRows = [object count];
        
            cellStyle = UITableViewCellStyleValue1;
        }
    }
    return self;
}



#pragma mark -
#pragma mark View lifecycle


- (void)viewDidLoad {
    [super viewDidLoad];
			  
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}


/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}
*/

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}



#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return numRows;
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    int index = [indexPath indexAtPosition:1];

    if (!isArray && !isDictionary)
        return nil;
    id obj = nil;
    NSString* text = nil;
    if (isArray)
    {
        text = [NSString stringWithFormat:@"item[%d]", index];
        obj = [object objectAtIndex:index];
    }
    else if (isDictionary)
    {
        NSArray* keys = [object allKeys];
        NSString* key = [keys objectAtIndex:index];
        text = [key description];
        obj = [object objectForKey:key];
    }
    NSString* CellIdentifier = [[obj class] description];
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil)
    {
        cell = [[[UITableViewCell alloc] initWithStyle:cellStyle reuseIdentifier:CellIdentifier] autorelease];
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    cell.textLabel.text = text;
    
    if ([obj isKindOfClass:[NSNumber class]])
    {
        cell.detailTextLabel.text = [obj stringValue];
    }
    else if ([obj isKindOfClass:[NSData class]] || [obj isKindOfClass:[NSDate class]])
    {
        cell.detailTextLabel.text = [obj description];
    }
    else if ([obj isKindOfClass:[NSDictionary class]])
    {
        cell.detailTextLabel.text = @"Dictionary";
        cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
        cell.selectionStyle = UITableViewCellSelectionStyleBlue;
    }
    else if ([obj isKindOfClass:[NSArray class]])
    {
        cell.detailTextLabel.text = @"Array";
        cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
        cell.selectionStyle = UITableViewCellSelectionStyleBlue;
    }
    else if ([obj isKindOfClass:[NSString class]])
    {
        cell.detailTextLabel.text = obj;
    }
    else
    {
        cell.detailTextLabel.text = [obj description];
    }
    return cell;
}


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	int index = [indexPath indexAtPosition:1];
	id obj = nil;
	NSString* title = nil;

	if (isArray)
	{
		obj = [object objectAtIndex:index];
		title = [NSString stringWithFormat:@"%@[%d]", self.title, index];
	}
	else if (isDictionary)
	{
		NSArray* keys = [object allKeys];
		id key = [keys objectAtIndex:index];
		obj = [object objectForKey:key];
		title = [key description];
	}
	if (obj == nil)
		return;
	
	if ([obj isKindOfClass:[NSArray class]] || [obj isKindOfClass:[NSDictionary class]])
	{
		GenericTableViewController* generic = [[GenericTableViewController alloc] initWithObject:obj];
		generic.title = title;
		[self.navigationController pushViewController:generic animated:YES];
		[generic release];
	}
}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath {
	[self tableView:tableView didSelectRowAtIndexPath:indexPath];
}
#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end

