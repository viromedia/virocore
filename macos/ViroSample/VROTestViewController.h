//
//  VROTestViewController.h
//  ViroSample
//
//  Created by Raj Advani on 10/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "VROTestRenderDelegate.h"
#import <ViroKit/ViroKit.h>

@interface VROTestViewController : NSViewController

@property (readwrite, nonatomic) IBOutlet VROTestRenderDelegate *renderDelegate;

@end
