//
//  VROImagePostProcess.h
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#ifndef VROImagePostProcess_h
#define VROImagePostProcess_h

#include <memory>
#include <vector>

class VRORenderTarget;
class VRODriver;
class VROTexture;

class VROImagePostProcess {
public:
    
    VROImagePostProcess() {}
    virtual ~VROImagePostProcess() {}
    
    /*
     Set to true to flip the result image vertically.
     */
    virtual void setVerticalFlip(bool flip) = 0;
    
    /*
     Bind the given textures and blit to the currently bound render target, using
     the post-process shader.
     
     The provided textures will be bound to samplers (texture units) 0 to N.
     */
    virtual void blit(std::vector<std::shared_ptr<VROTexture>> textures,
                      std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Prepare for a post-process that will use the same shader multiple times
     on different FBOs. This minimizes state changes by performing the
     configuration (binding the shader and vertex arrays) once for multiple
     blit operations.
     
     Used in conjunction with blitOpt() and end().
     */
    virtual void begin(std::shared_ptr<VRODriver> &driver) = 0;
    virtual void blitOpt(std::vector<std::shared_ptr<VROTexture>> textures,
                         std::shared_ptr<VRODriver> &driver) = 0;
    virtual void end(std::shared_ptr<VRODriver> &driver) = 0;
    
};

#endif /* VROImagePostProcess_h */
