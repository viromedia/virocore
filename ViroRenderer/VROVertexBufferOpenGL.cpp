//
//  VROVertexBufferOpenGL.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/29/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROVertexBufferOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VROAllocationTracker.h"

VROVertexBufferOpenGL::VROVertexBufferOpenGL(std::shared_ptr<VROData> data,
                                             std::shared_ptr<VRODriverOpenGL> driver) :
    VROVertexBuffer(data),
    _buffer(0),
    _driver(driver) {
    
}

VROVertexBufferOpenGL::~VROVertexBufferOpenGL() {
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (_buffer > 0 && driver) {
        driver->deleteBuffer(_buffer);
        _buffer = 0;
        
        ALLOCATION_TRACKER_SUB(VBO, 1);
    }
}

void VROVertexBufferOpenGL::hydrate() {
    if (_buffer != 0) {
        return;
    }
    
    GL( glGenBuffers(1, &_buffer) );
    GL( glBindBuffer(GL_ARRAY_BUFFER, _buffer) );
    GL( glBufferData(GL_ARRAY_BUFFER, _data->getDataLength(), _data->getData(), GL_STATIC_DRAW) );
    
    ALLOCATION_TRACKER_ADD(VBO, 1);
}
