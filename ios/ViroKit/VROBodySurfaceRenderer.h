//
//  VROBodySurfaceRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/29/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

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
