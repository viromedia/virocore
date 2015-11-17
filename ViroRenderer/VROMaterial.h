//
//  VROMaterial.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterial_h
#define VROMaterial_h

#include <memory>
#include "VROMaterialVisual.h"

class VROMaterial {
    
public:
    
    VROMaterial();
    virtual ~VROMaterial();
    
private:
    
    std::shared_ptr<VROMaterialVisual> _diffuse;
    std::shared_ptr<VROMaterialVisual> _ambient;
    std::shared_ptr<VROMaterialVisual> _specular;
    std::shared_ptr<VROMaterialVisual> _normal;
    std::shared_ptr<VROMaterialVisual> _reflective;
    std::shared_ptr<VROMaterialVisual> _emission;
    std::shared_ptr<VROMaterialVisual> _transparent;
    std::shared_ptr<VROMaterialVisual> _multiply;
    std::shared_ptr<VROMaterialVisual> _ambientOcclusion;
    std::shared_ptr<VROMaterialVisual> _selfIllumination;
    
};

#endif /* VROMaterial_h */
