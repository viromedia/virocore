//
//  VROViewScene.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/1/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import "VROView.h"

class VRORendererConfiguration;

@interface VROViewScene : NSView <VROView> 

- (instancetype)initWithFrame:(NSRect)frameRect
                       config:(VRORendererConfiguration)config
                 shareContext:(NSOpenGLContext *)context;

/*
 Base OpenGL view implementation.
 */
- (void)update;

/*
 Should be invoked before this object gets deallocated, to clean up GL
 resources on the rendering thread before the underlying OpenGL context used
 by this view is destroyed. This is required to prevent deadlocks in
 CVOpenGLTextureCache, which hangs on dealloc if the context it's using
 is already gone.
 */
- (void)deleteGL;

/*
 Sets the paused state of the underlying CADisplayLink
 */
- (void)setPaused:(BOOL)paused;

/*
 Set the background color for this view. This is the color that will be
 rendered behind the scene graph.
 */
- (void)setBackgroundColor:(NSColor *)color;

@end
