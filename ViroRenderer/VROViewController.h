//
//  VROViewController.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import "VRORenderContext.h"

typedef NS_ENUM(NSInteger, VROEyeType) {
    VROEyeTypeMonocular,
    VROEyeTypeLeft,
    VROEyeTypeRight,
};

@interface VROEyePerspective : NSObject

@property (nonatomic) VROEyeType type;

- (matrix_float4x4)eyeViewMatrix;
- (matrix_float4x4)perspectiveMatrixWithZNear:(float)zNear zFar:(float)zFar;

@end

@protocol VROStereoRendererDelegate <NSObject>

- (void)setupRendererWithView:(MTKView *)view context:(VRORenderContext *)context;
- (void)shutdownRendererWithView:(MTKView *)view;
- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context;

- (void)prepareNewFrameWithHeadViewMatrix:(matrix_float4x4)headViewMatrix;
- (void)renderEye:(VROEyePerspective *)eye context:(VRORenderContext *)renderContext;
- (void)finishFrameWithViewportRect:(CGRect)viewPort;

@optional

- (void)magneticTriggerPressed;

@end

@interface VROViewController : UIViewController <MTKViewDelegate>

@property (nonatomic) MTKView *view;
@property (nonatomic, readonly) NSRecursiveLock *glLock;

@property (nonatomic, unsafe_unretained) IBOutlet id <VROStereoRendererDelegate> stereoRendererDelegate;
@property (nonatomic) BOOL vrModeEnabled;
@property (nonatomic) BOOL distortionCorrectionEnabled;
@property (nonatomic) BOOL vignetteEnabled;
@property (nonatomic) BOOL chromaticAberrationCorrectionEnabled;
@property (nonatomic) BOOL restoreGLStateEnabled;
@property (nonatomic) BOOL neckModelEnabled;

@end
