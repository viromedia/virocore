//
//  VROPresentationLayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROPresentationLayer_h
#define VROPresentationLayer_h

#include "VROLayer.h"
#include "VROLayerSubstrate.h"
#include "VROByteBuffer.h"
#include "VROAnimation.h"
#include <memory>

class VROPresentationLayer : public VROLayer {
    
public:
    
    VROPresentationLayer(const VROLayer *layer, const VRORenderContext &context);
    virtual ~VROPresentationLayer();
    
    void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    void setContents(const void *data, size_t dataLength, int width, int height);
    
    void setFrame(VRORect frame);
    void setBounds(VRORect bounds);
    void setPosition(VROPoint point);

private:
    
    /*
     The corresponding model layer.
     */
    const VROLayer *_model;
    
    /*
     The representation of this layer in the underlying graphics technology.
     Responsible for rendering. Only the presentation layer has a substrate.
     */
    VROLayerSubstrate *_substrate;
    
    /*
     The contents of this layer, represented as formatted image data in a 
     byte buffer.
     */
    VROByteBuffer *_contents;
    
    /*
     For animations, the start values of each animatable property.
     */
    VRORect _animationStartFrame;
    
    /*
     The animation transaction this layer is participating in, if any.
     */
    std::shared_ptr<VROAnimation> _animation;
    
    void startFrameAnimation();
    void updateAnimatedFrame();
    
};

#endif /* VROPresentationLayer_h */
