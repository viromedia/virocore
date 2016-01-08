//
//  VROFrameListener.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROFrameListener_h
#define VROFrameListener_h

#include <stdio.h>

/*
 Interface for responding to frame begin and end events.
 */
class VROFrameListener {
    
public:
    
    virtual void onFrameWillRender() = 0;
    
    virtual void onFrameDidRender() = 0;
    
};

#endif /* VROFrameListener_h */
