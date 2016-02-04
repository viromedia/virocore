//
//  VROConcurrentBuffer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 2/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROConcurrentBuffer.h"

VROConcurrentBuffer::VROConcurrentBuffer(int size, NSString *label, id <MTLDevice> device) :
    _buffer([device newBufferWithLength:size * kMaxBuffersInFlight options:0]),
    _size(size) {
        
    _buffer.label = label;
}

VROConcurrentBuffer::~VROConcurrentBuffer() {
    
}