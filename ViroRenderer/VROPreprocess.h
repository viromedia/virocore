//
//  VROPreprocess.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROPreprocess_h
#define VROPreprocess_h

#include <stdio.h>
#include <memory>

class VROScene;
class VRODriver;
class VRORenderContext;

/*
 Tasks that are added to the VROChoreographer, which are run prior to rendering
 each frame. These processes are run after the frame is prepared, but prior to
 actual rendering; specifically, after updateSortKeys(), but before the choreographer
 renders any pass.
 */
class VROPreprocess {
public:
    
    VROPreprocess() {}
    virtual ~VROPreprocess() {}
    
    /*
     Execute the pre-process. The results can be stored in the given VRORenderContext,
     so that they are available to the full rendering system for the next pass.
     */
    virtual void execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                         std::shared_ptr<VRODriver> driver) = 0;
    
private:
    
};

#endif /* VROPreprocess_h */
