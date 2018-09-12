//
//  VRORenderUtil.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

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
