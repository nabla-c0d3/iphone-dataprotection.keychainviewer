//
//  RootViewController.m
//  KeychainViewer
//
//  Created by admin on 4/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RootViewController.h"
#import "ListViewController.h"
#import "GenericTableViewController.h"

#ifndef HGVERSION
#define HGVERSION "unknown"
#endif

@implementation RootViewController
@synthesize keychainCategories;
@synthesize keychain;
@synthesize about;

#pragma mark -
#pragma mark Initialization

/*
- (id)initWithStyle:(UITableViewStyle)style {
    // Override initWithStyle: if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
    self = [super initWithStyle:style];
    if (self) {
        
    }
    return self;
}
*/


#pragma mark -
#pragma mark View lifecycle


- (void)viewDidLoad {
    [super viewDidLoad];
    NSString* version = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString*)kCFBundleVersionKey];
    self.about = [NSDictionary dictionaryWithObjectsAndKeys:version, @"Version", @HGVERSION, @"Revision", @__DATE__, @"Build date", nil];
    self.keychainCategories = [NSArray arrayWithObjects:@"Generic Passwords",@"Internet Passwords",@"Certificates",@"Keys", @"About", nil];
    self.title = @"Keychain Viewer";
    self.keychain = keychain_open(NULL);
    if (self.keychain == NULL)
    {
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:nil message:@"Cannot open keychain database !" delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [alert show];
        [alert release];
    }
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

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
 }



#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.keychainCategories count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    }

    cell.textLabel.text = [self.keychainCategories objectAtIndex:[indexPath indexAtPosition:1]];
    if (self.keychain == NULL)
        cell.selectionStyle = UITableViewCellSelectionStyleNone;

    return cell;
}



#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	NSUInteger index = [indexPath indexAtPosition:1];
	
	if (index == 4) {
		GenericTableViewController* genericTable = [[GenericTableViewController alloc] initWithObject: self.about];
		genericTable.title = @"About";
		[self.navigationController pushViewController:genericTable animated:YES];
		[genericTable release];
		return;
	}
    if (self.keychain == NULL)
		return;
	ListViewController* listViewController = [[ListViewController alloc] initWithStyle:UITableViewStylePlain];
	
	if (index == 0) {
		listViewController.items = (NSArray*) keychain_get_passwords(self.keychain);
		listViewController.detailViewName = @"PasswordView";
	}
	else if (index == 1) {
		listViewController.items = (NSArray*) keychain_get_internet_passwords(self.keychain);
		listViewController.detailViewName = @"InternetPasswordView";
	}
	else if (index == 2) {
		listViewController.items = (NSArray*) keychain_get_certs(self.keychain);
		listViewController.detailViewName = @"CertificateView";
	}
	else if (index == 3) {
		listViewController.items = (NSArray*) keychain_get_keys(self.keychain);
		listViewController.detailViewName = @"KeyView";
	}
	listViewController.title = [self.keychainCategories objectAtIndex: index];
    [self.navigationController pushViewController:listViewController animated:YES];
    [listViewController release];
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

