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


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
		cell.accessoryType = UITableViewCellAccessoryDetailDisclosureButton;
    }
    NSDictionary* dict = [self.items objectAtIndex:[indexPath indexAtPosition:1]];
	
    if (![[dict objectForKey:@"title"] isKindOfClass:[NSString class]])
    {
        CFShow([dict objectForKey:@"title"]);
        return cell;
    }
	cell.textLabel.text = [dict objectForKey:@"title"];
	if ([dict objectForKey:@"subtitle"])
		cell.detailTextLabel.text = [dict objectForKey:@"subtitle"];
    
    if ([dict objectForKey:@"safe_protection_class"]) {
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
	
	keychain_process(dict);
	detailViewController.data = dict;
    NSString* title = [[self.items objectAtIndex:[indexPath indexAtPosition:1]] objectForKey:@"title"];
    if([title isKindOfClass:[NSString class]])
        detailViewController.title = title;
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

