//
//  KeychainViewerAppDelegate.h
//  KeychainViewer
//
//  Created by admin on 4/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface KeychainViewerAppDelegate : NSObject <UIApplicationDelegate> {
    
    UIWindow *window;
    UINavigationController *navigationController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;

@end

