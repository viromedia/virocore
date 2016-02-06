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
#import "VROScreenUIView.h"
#import <memory>

class VROScene;

@interface VROView : MTKView <MTKViewDelegate>

@property (nonatomic, unsafe_unretained) IBOutlet id <VRORenderDelegate> renderDelegate;
@property (nonatomic) BOOL vrModeEnabled;
@property (nonatomic) BOOL distortionCorrectionEnabled;
@property (nonatomic) BOOL vignetteEnabled;
@property (nonatomic) BOOL chromaticAberrationCorrectionEnabled;

@property (nonatomic, readonly) VROScreenUIView *HUD;
@property (readwrite, nonatomic) std::shared_ptr<VROScene> scene;

- (instancetype)initWithFrame:(CGRect)frame;
- (VRORenderContext *)renderContext;

@end
