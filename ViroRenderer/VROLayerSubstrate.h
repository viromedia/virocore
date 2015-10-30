//
//  VROLayerSubstrate.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/20/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROLayerSubstrate_h
#define VROLayerSubstrate_h

#include <memory>
#include <stack>
#include <simd/simd.h>
#include "SharedStructures.h"

class VROLayer;
class VRORenderContext;

extern size_t kCornersInLayer;

/*
 The representation of a layer in the underlying graphics technology. Responsible
 for rendering its parent layer.
 */
class VROLayerSubstrate {
    
public:
    
    VROLayerSubstrate() {}
    virtual ~VROLayerSubstrate() {}
    
    virtual void render(const VRORenderContext &context,
                        matrix_float4x4 mv,
                        vector_float4 bgColor) = 0;
    virtual void setContents(const void *data, const size_t dataLength, int width, int height) = 0;

protected:
    
    void buildQuad(VROLayerVertexLayout *layout);
    
};

#endif /* VROLayerSubstrate_h */
