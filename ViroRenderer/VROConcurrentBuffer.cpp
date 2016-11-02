//
//  VROConcurrentBuffer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 2/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROConcurrentBuffer.h"
#if VRO_METAL

VROConcurrentBuffer::VROConcurrentBuffer(int size, NSString *label, id <MTLDevice> device) :
    _size(size) {
        
    _buffer[(int)VROEyeType::Left]  = [device newBufferWithLength:size * kMaxBuffersInFlight options:0];
    _buffer[(int)VROEyeType::Right] = [device newBufferWithLength:size * kMaxBuffersInFlight options:0];

    _buffer[(int)VROEyeType::Left].label = [label stringByAppendingString:@"-Left"];
    _buffer[(int)VROEyeType::Right].label = [label stringByAppendingString:@"-Right"];
}

VROConcurrentBuffer::~VROConcurrentBuffer() {
    
}

#endif