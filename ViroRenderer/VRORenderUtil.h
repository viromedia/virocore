//
//  VRORenderUtil.h
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

#ifndef VRORenderUtil_h
#define VRORenderUtil_h

#include <stdio.h>
#include <memory>
class VROTexture;
class VRODriver;

/*
 Utilities for rendering simple objects, used by render passes.
 */
class VRORenderUtil {
public:

    /*
     Set all properties in the driver required for successful blit operations.
     */
    static void prepareForBlit(std::shared_ptr<VRODriver> &driver, bool enableDepth,
                               bool enableStencil);
    
    /*
     Render a unit cube. The cube will be rendered using the given VAO
     on top of the given VBO. If the VAO/VBO do not yet exist (are 0),
     they will be generated.
     */
    static void renderUnitCube(unsigned int *vao, unsigned int *vbo);

    /*
     Renders a unit quad. The quad will be rendered using the given VAO
     on top of the given VBO. If the VAO/VBO do not yet exist (are 0),
     they will be generated.
     */
    static void renderQuad(unsigned int *vao, unsigned int *vbo);

    /*
     Bind the given texture to the given texture unit. Returns true on
     success.
     */
    static bool bindTexture(int unit, const std::shared_ptr<VROTexture> &texture,
                            std::shared_ptr<VRODriver> &driver);
    
};

#endif /* VRORenderUtil_h */
