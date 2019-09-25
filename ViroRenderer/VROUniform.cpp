//
//  VROUniform.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/14/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

void VROUniformBinder::setForMaterial(VROUniform *uniform, const VROGeometry *geometry, const VROMaterial *material) {
    if (uniform->getLocation() == -1) {
        return;
    }
    _bindingBlock(uniform, geometry, material);
}
