//
//  RootViewController.m
//  KeychainViewer
//
//  Created by admin on 4/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ListViewController.h"
#import "DetailViewController.h"

@implementation ListViewController

@synthesize items;
@synthesize detailViewName;
#pragma mark -
#pragma mark View lifecycle


- (void)viewDidLoad {
    [super viewDidLoad];
	
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}



- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
	
}

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

// Customize the number of sections in the table view.
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.items count];
}

NSString* convertToString(id obj)
{
    if ([obj isKindOfClass:[NSString class]])
        return obj;
    return [obj description];
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
		cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
    }
    NSDictionary* dict = [self.items objectAtIndex:[indexPath indexAtPosition:1]];
	
    if ([dict objectForKey:@"common_name"])
    {
        cell.textLabel.text = [dict objectForKey:@"common_name"];
        cell.detailTextLabel.text = convertToString([dict objectForKey:@"labl"]);
    }
    else if ([dict objectForKey:@"srvr"])
    {
        cell.textLabel.text = convertToString([dict objectForKey:@"acct"]);
        cell.detailTextLabel.text = convertToString([dict objectForKey:@"srvr"]);
    }
    else
    {
        cell.textLabel.text = convertToString([dict objectForKey:@"acct"]);
        cell.detailTextLabel.text = convertToString([dict objectForKey:@"svce"]);
    }
    if (![cell.textLabel.text length]) {
        cell.textLabel.text = convertToString([dict objectForKey:@"labl"]);
    }
    if (![cell.detailTextLabel.text length]) {
        cell.detailTextLabel.text = convertToString([dict objectForKey:@"agrp"]);
    }
    
    NSString* clas = (NSString*) [dict objectForKey:@"protection_class"];

    if (![clas hasPrefix:@"Always"]) {
        cell.textLabel.textColor = [UIColor greenColor];
    }
    else {
        //dequeueReusableCellWithIdentifier can give us back a cell with the other text color
        cell.textLabel.textColor = [UIColor blackColor];
    }

    return cell;
}

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	DetailViewController *detailViewController = [[DetailViewController alloc] initWithPlist:self.detailViewName];
	
	NSDictionary* dict = [self.items objectAtIndex:[indexPath indexAtPosition:1]];
	UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
	detailViewController.data = dict;

    if(cell != nil)
        detailViewController.title = cell.textLabel.text;
	[self.navigationController pushViewController:detailViewController animated:YES];
	[detailViewController release];
}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath {
	[self tableView:tableView didSelectRowAtIndexPath:indexPath];
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end

