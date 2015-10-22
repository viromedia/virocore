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

class VROPresentationLayer : public VROLayer {
    
public:
    
    VROPresentationLayer(const VROLayer *layer);
    virtual ~VROPresentationLayer();
    
    void hydrate(const VRORenderContext &context);
    void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    
    void setContents(const void *data, size_t dataLength, int width, int height);

private:
    
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
    
};

#endif /* VROPresentationLayer_h */
