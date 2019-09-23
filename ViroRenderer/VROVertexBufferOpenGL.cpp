//
//  VROVertexBufferOpenGL.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/29/19.
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
    if (_buffer > 0) {
        if (driver) {
            driver->deleteBuffer(_buffer);
        }
        _buffer = 0;
        
        // Even if the driver was released we subtract the VBO, because a released
        // driver implies the entire GL context (including all VBOs) were released.
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
