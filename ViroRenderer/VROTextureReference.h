//
//  VROTextureReference.h
//  ViroKit
//
//  Created by Raj Advani on 1/24/18.
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

#ifndef VROTextureReference_h
#define VROTextureReference_h

#include <memory>

class VROTexture;
class VRORenderContext;

enum VROGlobalTextureType {
    ShadowMap,
    IrradianceMap,
    PrefilteredMap,
    BrdfMap
};

/*
 References a local texture directly, or a global texture (e.g. shadow map,
 irradiance map, etc.) indirectly via an identifier.
 
 Local textures are textures held by individual materials, while global textures
 are stored in the VRORenderContext for the frame.
 */
class VROTextureReference {
public:
    VROTextureReference(std::shared_ptr<VROTexture> localTexture) : _isGlobal(false), _localTexture(localTexture) {}
    VROTextureReference(VROGlobalTextureType globalType) : _isGlobal(true), _globalType(globalType) {}
    
    bool isGlobal() const { return _isGlobal; }
    VROGlobalTextureType getGlobalType() const { return _globalType; }

    std::shared_ptr<VROTexture> getTexture(const VRORenderContext &context) const {
        return _isGlobal ? getGlobalTexture(context) : getLocalTexture();
    };
    
    std::shared_ptr<VROTexture> getLocalTexture() const { return _localTexture; }
    std::shared_ptr<VROTexture> getGlobalTexture(const VRORenderContext &context) const;
    
private:
    bool _isGlobal;
    VROGlobalTextureType _globalType;
    std::shared_ptr<VROTexture> _localTexture;
    
};

#endif /* VROTextureReference_h */
