//
//  VROVertexBuffer.h
//  ViroKit
//
//  Created by Raj Advani on 6/29/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROVertexBuffer_h
#define VROVertexBuffer_h

#include <stdio.h>
#include "VROData.h"

class VROVertexBuffer {
public:
    
    VROVertexBuffer(std::shared_ptr<VROData> data) :
        _data(data) {}
    virtual ~VROVertexBuffer() {}
    
    /*
     Upload this buffer to the GPU. No-op if this buffer is already on the GPU.
     */
    virtual void hydrate() = 0;
    
    /*
     Get the data (on the CPU) underlying this vertex buffer.
     */
    std::shared_ptr<VROData> getData() const { return _data; }
    
protected:
    
    std::shared_ptr<VROData> _data;
    
};

#endif /* VROVertexBuffer_h */
