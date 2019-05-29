//
//  VROBodySurfaceRenderer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/29/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBodySurfaceRenderer.h"
#include "VROBodyMesheriOS.h"

VROBodySurfaceRenderer::VROBodySurfaceRenderer(VROViewAR *view, std::shared_ptr<VROBodyMesher> mesher) {
    _view = view;
    _bodyMesher = mesher;
    _drawDelegate = [[VROBodyMesherDrawDelegate alloc] init];
    [_view setDebugDrawDelegate:_drawDelegate];
}

VROBodySurfaceRenderer::~VROBodySurfaceRenderer() {
    
}

void VROBodySurfaceRenderer::onBodyMeshUpdated(const std::vector<float> &vertices, std::shared_ptr<VROGeometry> mesh) {
    int viewWidth  = _view.frame.size.width;
    int viewHeight = _view.frame.size.height;
    
    std::shared_ptr<VROBodyMesher> tracker = _bodyMesher.lock();
    //if (tracker) {
    //    std::shared_ptr<VROBodyTrackeriOS> trackeriOS = std::dynamic_pointer_cast<VROBodyTrackeriOS>(tracker);
    //    [_drawDelegate setDynamicCropBox:trackeriOS->getDynamicCropBox()];
    //}
    [_drawDelegate setVertices:vertices];
    [_drawDelegate setViewWidth:viewWidth height:viewHeight];
}

@implementation VROBodyMesherDrawDelegate {
    std::vector<float> _vertices;
    CGRect _dynamicCropBox;
    int _viewWidth, _viewHeight;
}

- (id)init {
    self = [super init];
    if (self) {
        _dynamicCropBox = CGRectNull;
    }
    return self;
}

- (void)drawRect {
    CGContextRef context = UIGraphicsGetCurrentContext();
    UIFont *font = [UIFont boldSystemFontOfSize:16];
    
    CGContextSetRGBFillColor(context, 0, 1, 0, 1);
    CGContextSetRGBStrokeColor(context, 0, 1, 0, 1);
    CGContextSetLineWidth(context, 3);
    
    if (!CGRectIsNull(_dynamicCropBox)) {
        CGFloat red = 0.0, green = 1.0, blue = 0.0, alpha = 1.0;
        CGContextSetRGBStrokeColor(context, red, green, blue, alpha);
        
        CGContextAddRect(context, CGRectApplyAffineTransform(_dynamicCropBox, CGAffineTransformMakeScale(_viewWidth, _viewHeight)));
        CGContextStrokePath(context);
    }
    
    for (int i = 0; i < _vertices.size() / 3; i++) {
        float x = _vertices[i * 3 + 0] * _viewWidth;
        float y = (1 - _vertices[i * 3 + 1]) * _viewHeight;
        //float z = _vertices[i * 3 + 2];
        
        CGFloat red = 1.0, green = 0.0, blue = 0.0;
        CGContextSetRGBFillColor(context, red, green, blue, 1);
        CGContextSetRGBStrokeColor(context, red, green, blue, 1);
        CGContextSetLineWidth(context, 1);
        
        float radius = 1;
        CGRect rect = CGRectMake(x - radius, y - radius, radius * 2, radius * 2);
        CGContextAddEllipseInRect(context, rect);
        CGContextDrawPath(context, kCGPathFillStroke);
    }
}

- (void)setVertices:(std::vector<float>)vertices {
    _vertices = vertices;
}
- (void)setDynamicCropBox:(CGRect)box {
    _dynamicCropBox = box;
}
- (void)setViewWidth:(int)width height:(int)height {
    _viewWidth = width;
    _viewHeight = height;
}

@end
