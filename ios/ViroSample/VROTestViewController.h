//
//  VROTestViewController.h
//  ViroSample
//
//  Created by Raj Advani on 10/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VROTestRenderDelegate.h"
#import <ViroKit/ViroKit.h>

@interface VROTestViewController : UIViewController

@property (readwrite, nonatomic) IBOutlet VROTestRenderDelegate *renderDelegate;

@end
