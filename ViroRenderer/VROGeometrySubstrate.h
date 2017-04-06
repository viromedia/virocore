//
//  VROGeometrySubstrate.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

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

/*
 Represents the geometry in the underlying graphics hardware.
 */
class VROGeometrySubstrate {
    
public:
    
    virtual ~VROGeometrySubstrate() {}
    
    virtual void render(const VROGeometry &geometry,
                        int elementIndex,
                        VROMatrix4f transform,
                        VROMatrix4f normalMatrix,
                        float opacity,
                        std::shared_ptr<VROMaterial> &material,
                        const VRORenderContext &context,
                        std::shared_ptr<VRODriver> &driver) = 0;
};

#endif /* VROGeometrySubstrate_h */
