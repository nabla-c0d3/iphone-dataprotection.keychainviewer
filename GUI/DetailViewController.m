//
//  PasswordViewController.m
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "DetailViewController.h"
#import "GenericTableViewController.h"

@implementation DetailViewController

@synthesize description;
@synthesize data;

#pragma mark -
#pragma mark Initialization


- (id)initWithPlist:(NSString*) plist {
    self = [super initWithStyle:UITableViewStyleGrouped];
    if (self) {
		NSString* plistPath = [[NSBundle mainBundle] pathForResource:plist ofType:@"plist"];
        self.description = [NSArray arrayWithContentsOfFile:plistPath];
		//self.tableView.allowsSelection = NO;
    }
    return self;
}



#pragma mark -
#pragma mark View lifecycle

/*
- (void)viewDidLoad {
    [super viewDidLoad];

    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}
*/

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

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}


#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return [self.description count];
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [[[self.description objectAtIndex:section] objectForKey:@"rows"] count];
}

- (NSString*) tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger) section {
	return [[self.description objectAtIndex:section] objectForKey:@"title"];
}



// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:CellIdentifier] autorelease];
		cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    
	NSString* key = [[[self.description objectAtIndex:[indexPath indexAtPosition:0]] objectForKey:@"rows"] objectAtIndex: [indexPath indexAtPosition:1]];
    cell.textLabel.text = NSLocalizedString(key, key);
	
	id obj = [self.data objectForKey:key];
	
    if ([obj isKindOfClass:[NSNull class]])
    {
        obj = @"[NULL]";
    }
    else if ([obj isKindOfClass:[NSDictionary class]])
    {
        obj = @"Dictionary";
        cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
        cell.selectionStyle = UITableViewCellSelectionStyleBlue;
    }
    else if ([obj isKindOfClass:[NSArray class]])
    {
        obj = @"Array";
        cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
        cell.selectionStyle = UITableViewCellSelectionStyleBlue;
    }
    else if ([obj isKindOfClass:[NSNumber class]])
    {
        obj = [obj stringValue];
    }
    else if (![obj isKindOfClass:[NSString class]])
    {
        obj = [obj description];
    }
    cell.detailTextLabel.text = obj;
    
    return cell;
}

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
	NSString* key = [[[self.description objectAtIndex:[indexPath indexAtPosition:0]] objectForKey:@"rows"] objectAtIndex: [indexPath indexAtPosition:1]];
	id obj = [self.data objectForKey:key];
	
	if ([obj isKindOfClass:[NSDictionary class]] || [obj isKindOfClass:[NSArray class]])
	{
		GenericTableViewController* genericTable = [[GenericTableViewController alloc] initWithObject:obj];
		genericTable.title = self.title;
        [self.navigationController pushViewController:genericTable animated:YES];
		[genericTable release];
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

