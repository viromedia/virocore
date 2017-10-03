//
//  VROSample.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <ViroKit/ViroKit.h>

@interface VROTestRenderDelegate : NSObject <VRORenderDelegate>

@property (readwrite, nonatomic) IBOutlet id <VROView> view;

- (IBAction)nextScene:(id)sender;

@end


