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
#include "VROLayerSubstrate.h"
#include "SharedStructures.h"
#include "VRORect.h"
#include <vector>
#include <stack>
#include <memory>

class VROLayer : public std::enable_shared_from_this<VROLayer> {
    
public:
    
    VROLayer();
    VROLayer(bool presentation);
    
    virtual ~VROLayer();
    
    void hydrate(const VRORenderContext &context);
    void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    
    void setFrame(VRORect frame);
    void setBounds(VRORect bounds);
    void setPosition(VROPoint point);
    
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
    
private:
    
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
    
    /*
     The 'presentation' counterpart of this layer. The presentation layer
     reflects what's actually on the screen during an animation, as opposed
     to the model.
     */
    std::shared_ptr<VROLayer> _presentationLayer;
    
    /*
     The representation of this layer in the underlying graphics technology.
     Responsible for rendering. Only the presentation layer has a substrate.
     */
    VROLayerSubstrate *_substrate;
    
};

#endif /* VROLayer_h */
