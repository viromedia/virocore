//
//  VROGeometrySubstrate.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#ifndef VROGeometrySubstrate_h
#define VROGeometrySubstrate_h

#include <stdio.h>
#include <vector>
#include <memory>

class VROLight;
class VRORenderContext;
class VRODriver;
class VROMatrix4f;
class VROMaterial;
class VROGeometry;
class VROMaterial;
class VROTexture;

/*
 Represents the geometry in the underlying graphics hardware.
 */
class VROGeometrySubstrate {
    
public:
    
    virtual ~VROGeometrySubstrate() {}
    
    /*
     Update any per-frame data in this substrate. Invoked once per frame
     during the updateSortKeys() phase.
     */
    virtual void update(const VROGeometry &geometry,
                        std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Render the given element of the geometry with full texturing and
     lighting. Assumes the material's shader and geometry-independent
     properties have already been bound.
     */
    virtual void render(const VROGeometry &geometry,
                        int elementIndex,
                        VROMatrix4f transform,
                        VROMatrix4f normalMatrix,
                        float opacity,
                        const std::shared_ptr<VROMaterial> &material,
                        const VRORenderContext &context,
                        std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Render the silhouette of the entire geometry (all elements). Renders
     using the given material, which is assumed to already be bound, ignoring
     texturing and lighting. Typically this is used for rendering to a stencil
     buffer or shadow map.
     */
    virtual void renderSilhouette(const VROGeometry &geometry,
                                  VROMatrix4f transform,
                                  std::shared_ptr<VROMaterial> &material,
                                  const VRORenderContext &context,
                                  std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Render the silhouette of the given element of the given geometry.
     Renders using the provided material, which is assumed to already be
     bound, and binds its associated diffuse texture.
     */
    virtual void renderSilhouetteTextured(const VROGeometry &geometry,
                                          int element,
                                          VROMatrix4f transform,
                                          std::shared_ptr<VROMaterial> &material,
                                          const VRORenderContext &context,
                                          std::shared_ptr<VRODriver> &driver) = 0;
};

#endif /* VROGeometrySubstrate_h */
