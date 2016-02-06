//
//  GameViewController.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ViroKit/ViroKit.h>

@interface SampleRenderer : NSObject <VRORenderDelegate>

- (IBAction)nextScene:(id)sender;

@end


