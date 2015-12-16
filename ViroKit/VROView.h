//
//  VROView.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "VRORenderDelegate.h"

@interface VROView : MTKView <MTKViewDelegate>

@property (nonatomic, unsafe_unretained) IBOutlet id <VRORenderDelegate> renderDelegate;
@property (nonatomic) BOOL vrModeEnabled;
@property (nonatomic) BOOL distortionCorrectionEnabled;
@property (nonatomic) BOOL vignetteEnabled;
@property (nonatomic) BOOL chromaticAberrationCorrectionEnabled;

- (instancetype)initWithFrame:(CGRect)frame;
- (VRORenderContext *)renderContext;

@end
