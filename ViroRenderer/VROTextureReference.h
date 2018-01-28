//
//  VROTextureReference.h
//  ViroKit
//
//  Created by Raj Advani on 1/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

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
