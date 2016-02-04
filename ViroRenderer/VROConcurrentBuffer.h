//
//  VROConcurrentBuffer.h
//  ViroRenderer
//
//  Created by Raj Advani on 2/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROConcurrentBuffer_h
#define VROConcurrentBuffer_h

#include <stdio.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

static const int kMaxBuffersInFlight = 3;

/*
 Wraps a Metal buffer that's used by both the GPU and the CPU.
 The section of the buffer being used (written to by the CPU) changes
 with each frame, so that the CPU and GPU never collide.
 */
class VROConcurrentBuffer {
    
public:
    
    VROConcurrentBuffer(int size, NSString *label, id <MTLDevice> device);
    virtual ~VROConcurrentBuffer();
    
    /*
     Get the underlying MTLBuffer.
     */
    id <MTLBuffer> getMTLBuffer() {
        return _buffer;
    }
    
    void *getWritableContents(int frame) {
        return (void *) ((char *)[_buffer contents] + getWriteOffset(frame));
    }
    
    /*
     Get the CPU write offset into the MTLBuffer, based on the current
     frame.
     */
    int getWriteOffset(int frame) {
        return _size * (frame % kMaxBuffersInFlight);
    }
    
private:
    
    /*
     The underlying Metal buffer.
     */
    id <MTLBuffer> _buffer;
    
    /*
     The size of each section in the MTLBuffer.
     */
    int _size;
    
};

#endif /* VROConcurrentBuffer_h */
