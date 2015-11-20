//
//  VROLayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROLayer_h
#define VROLayer_h

#include <stdio.h>
#include <simd/simd.h>
#include "VRORenderContext.h"
#include "SharedStructures.h"
#include "VRORect.h"
#include <vector>
#include <stack>
#include <memory>
#include "VROByteBuffer.h"
#include "VROGeometry.h"

class VROLayer : public std::enable_shared_from_this<VROLayer> {
    
public:
    
    /*
     Designated initializer for layers in the model tree.
     */
    VROLayer(const VRORenderContext &context);
    
    /*
     Designated initializer for layers in the presentation tree.
     */
    VROLayer(VROLayer *layer);
    virtual ~VROLayer();
    
    virtual void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    virtual void setContents(const void *data, size_t dataLength, size_t width, size_t height);
    
    virtual void setFrame(VRORect frame);
    virtual void setBounds(VRORect bounds);
    virtual void setPosition(VROPoint point);
    
    VRORect getFrame() const;
    VRORect getBounds() const;
    VROPoint getPosition() const;
    
    std::shared_ptr<VROLayer> getSuperlayer() const {
        return _superlayer;
    }
    
    void setBackgroundColor(vector_float4 backgroundColor);
    vector_float4 getBackgroundColor() const;
    
    void addSublayer(std::shared_ptr<VROLayer> layer);
    void removeFromSuperlayer();
    
protected:
    
    /*
     Position and size of the layer in 3D space or in its parent layer's
     2D space.
     */
    VRORect _frame;
    
    /*
     The color of the layer.
     */
    vector_float4 _backgroundColor;
    
    /*
     The layer's parent and children.
     */
    std::vector<std::shared_ptr<VROLayer>> _sublayers;
    std::shared_ptr<VROLayer> _superlayer;
    
private:
    
    /*
     The 'presentation' counterpart of this layer. The presentation layer
     reflects what's actually on the screen during an animation, as opposed
     to the model.
     */
    std::shared_ptr<VROLayer> _presentationLayer;
    
    /*
     The geometry used by the renderer to draw this layer.
     */
    std::shared_ptr<VROGeometry> _geometry;
    
};

#endif /* VROLayer_h */
