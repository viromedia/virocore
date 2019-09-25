//
//  VROPreprocess.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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
