//
//  VROVertexBuffer.h
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
