//
//  VROBodySurfaceRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/29/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROBodySurfaceRenderer_h
#define VROBodySurfaceRenderer_h

#include "VROBodyMesher.h"
#import "VROViewAR.h"
#import <UIKit/UIKit.h>

@interface VROBodyMesherDrawDelegate : NSObject<VRODebugDrawDelegate>
- (void)drawRect;
- (void)setVertices:(std::vector<float>)vertices;
- (void)setDynamicCropBox:(CGRect)box;
- (void)setViewWidth:(int)width height:(int)height;
@end

/*
 Renders the body surface to the screen.
 */
class VROBodySurfaceRenderer : public VROBodyMesherDelegate {
    
public:
    
    VROBodySurfaceRenderer(VROViewAR *view, std::shared_ptr<VROBodyMesher> mesher);
    virtual ~VROBodySurfaceRenderer();
    
    void onBodyMeshUpdated(const std::vector<float> &vertices, std::shared_ptr<VROGeometry> mesh);
    
private:
    
    VROViewAR *_view;
    std::weak_ptr<VROBodyMesher> _bodyMesher;
    VROBodyMesherDrawDelegate *_drawDelegate;
    
};

#endif /* VROBodySurfaceRenderer_h */
