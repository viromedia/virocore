//
//  VROViewAR.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>
#import "VROView.h"
#import "VROARSession.h"
#import "VROARHitTestResult.h"

class VROARSessionDelegate;
class VRORendererConfiguration;

@protocol VRODebugDrawDelegate
@required
- (void)drawRect;
@end

@interface VROGlassView : UIView
@property (readwrite, nonatomic, assign) NSObject<VRODebugDrawDelegate> *debugDrawDelegate;
- (id)initWithFrame:(CGRect)frame delegate:(NSObject<VRODebugDrawDelegate> *)delegate;
@end

@interface VROViewAR : GLKView <VROView, UIGestureRecognizerDelegate>

@property (readwrite, nonatomic) BOOL suspended;

/*
 True if the X dimension is mirrored on the device. This is typically true when
 running a front-facing camera.
 */
@property (readonly, nonatomic) BOOL mirrored;

- (instancetype)initWithFrame:(CGRect)frame
                       config:(VRORendererConfiguration)config
                      context:(EAGLContext *)context
               worldAlignment:(VROWorldAlignment)worldAlignment;

- (instancetype)initWithFrame:(CGRect)frame
                       config:(VRORendererConfiguration)config
                      context:(EAGLContext *)context
               worldAlignment:(VROWorldAlignment)worldAlignment
                 trackingType:(VROTrackingType)trackingType;

- (void)setARSessionDelegate:(std::shared_ptr<VROARSessionDelegate>)delegate;

/*
 Should be invoked before this object gets deallocated, to clean up GL
 resources on the rendering thread before the underlying EAGLContext used
 by this view is destroyed. This is required to prevent deadlocks in
 CVOpenGLTextureCache, which hangs on dealloc if the EAGLContext it's using
 is already gone.
 */
- (void)deleteGL;

/*
 Sets the paused state of the underlying CADisplayLink
 */
- (void)setPaused:(BOOL)paused;

/*
 Performs an AR hit test with the given ray assuming origin is the camera
 */
- (std::vector<std::shared_ptr<VROARHitTestResult>>)performARHitTest:(VROVector3f)ray;

/*
 Performs an AR hit test with the given 2D point on the screen
 */
- (std::vector<std::shared_ptr<VROARHitTestResult>>)performARHitTestWithPoint:(int)x y:(int)y;

/*
 Returns the ARSession
 */
- (std::shared_ptr<VROARSession>)getARSession;

/*
 Returns true if AR is supported by this device.
 */
+ (BOOL)isARSupported;

/*
 Set a view for drawing debug information using CoreGraphics.
 */
- (void)setDebugDrawDelegate:(NSObject<VRODebugDrawDelegate> *)debugDrawDelegate;

@end
