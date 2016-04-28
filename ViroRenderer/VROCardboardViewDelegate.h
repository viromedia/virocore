//
//  VROCardboardViewDelegate.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GCSCardboardView.h"
#import <memory>

class VRORenderer;

@interface VROCardboardViewDelegate : NSObject <GCSCardboardViewDelegate>

- (id)initWithRenderer:(std::shared_ptr<VRORenderer>)renderer;

/**
 * Called when a user event is fired. See the documentation of |GCSUserEvent| to find out what
 * thread is used to make this call.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView didFireEvent:(GCSUserEvent)event;

/**
 * Called before the first draw frame call. This is called on the GL thread and can be used to do
 * any pre-rendering setup required while on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
     willStartDrawing:(GCSHeadTransform *)headTransform;

/**
 * Called at the start of each frame, before calling both eyes. Delegate should use initialize or
 * clear the GL state. This method is called on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
     prepareDrawFrame:(GCSHeadTransform *)headTransform;

/**
 * Called on each frame to perform the required GL rendering. Delegate should set the GL viewport
 * and scissor it to the viewport returned from |GCSHeadTransforms|'s |viewportForEye| method.
 * This method is called on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
              drawEye:(GCSEye)eye
    withHeadTransform:(GCSHeadTransform *)headTransform;

@end
