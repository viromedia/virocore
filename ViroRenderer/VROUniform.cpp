//
//  VROUniform.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/14/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROUniform.h"
#include "VROShaderModifier.h"

VROUniform *VROUniform::newUniformForType(const std::string &name, VROShaderProperty type, int arraySize) {
    VROUniform *uniform = nullptr;
    
    switch (type) {
        case VROShaderProperty::Int:
            uniform = new VROUniform1i(name, arraySize);
            break;
            
        case VROShaderProperty::IVec2:
            uniform = new VROUniform2i(name, arraySize);
            break;
            
        case VROShaderProperty::IVec3:
            uniform = new VROUniform3i(name, arraySize);
            break;
            
        case VROShaderProperty::IVec4:
            uniform = new VROUniform4i(name, arraySize);
            break;
            
        case VROShaderProperty::Float:
            uniform = new VROUniform1f(name, arraySize);
            break;
            
        case VROShaderProperty::Vec2:
            uniform = new VROUniform2f(name, arraySize);
            break;
            
        case VROShaderProperty::Vec3:
            uniform = new VROUniform3f(name, arraySize);
            break;
            
        case VROShaderProperty::Vec4:
            uniform = new VROUniform4f(name, arraySize);
            break;
            
        case VROShaderProperty::Mat2:
            uniform = new VROUniformMat2(name);
            break;
            
        case VROShaderProperty::Mat3:
            uniform = new VROUniformMat3(name);
            break;
            
        case VROShaderProperty::Mat4:
            uniform = new VROUniformMat4(name);
            break;
            
        default:
            pabort("Unsupported shader uniform type VROShaderProperty: %d", static_cast<int>(type));
            break;
    }
    
    return uniform;
}

void VROUniformShaderModifier::set(const void *value, const VROGeometry *geometry, const VROMaterial *material) {
    if (_location == -1) {
        return;
    }
    
    _modifier->bindUniform(this, _location, geometry, material);
}
