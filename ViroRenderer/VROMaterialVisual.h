//
//  VROMaterialVisual.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterialVisual_h
#define VROMaterialVisual_h

#include "VROVector4f.h"
#include "VROMatrix4f.h"
#include "VROTexture.h"
#include <vector>

enum class VROWrapMode {
    Clamp,
    Repeat,
    ClampToBorder,
    Mirror
};

enum class VROFilterMode {
    None,
    Nearest,
    Linear
};

enum class VROContentsType {
    Fixed,
    Texture2D,
    TextureCube
};

class VROMaterialVisual {
    
public:
    
    VROMaterialVisual() :
        _contentsType(VROContentsType::Fixed),
        _contentsColor({ 1.0, 1.0, 1.0, 1.0 }),
        _intensity(1.0),
        _wrapS(VROWrapMode::Clamp),
        _wrapT(VROWrapMode::Clamp),
        _minificationFilter(VROFilterMode::Linear),
        _magnificationFilter(VROFilterMode::Linear),
        _mipFilter(VROFilterMode::None)
    {}
    
    void setContents(VROVector4f contents) {
        _contentsColor = contents;
        _contentsType = VROContentsType::Fixed;
    }
    
    void setContents(std::shared_ptr<VROTexture> texture) {
        _contentsTexture = texture;
        _contentsType = VROContentsType::Texture2D;
    }
    
    void setContents(std::vector<std::shared_ptr<VROTexture>> cubeTextures) {
        _contentsCube = cubeTextures;
        _contentsType = VROContentsType::TextureCube;
    }
    
    VROContentsType getContentsType() const {
        return _contentsType;
    }
    
    VROVector4f getContentsColor() const {
        if (_contentsType == VROContentsType::Fixed) {
            return _contentsColor;
        }
        else {
            return { 1.0, 1.0, 1.0, 1.0 };
        }
    }
    
    std::shared_ptr<VROTexture> getContentsTexture() const {
        if (_contentsType == VROContentsType::Texture2D) {
            return _contentsTexture;
        }
        else {
            return {};
        }
    }
    
private:
    
    /*
     Indicates the content type for this visual.
     */
    VROContentsType _contentsType;
    
    /*
     If the visual is determined by a fixed color, _contentsColor is populated.
     */
    VROVector4f _contentsColor;
    
    /*
     If the visual is determiend by a texture, this variable will be populated
     with the texture.
     */
    std::shared_ptr<VROTexture> _contentsTexture;
    
    /*
     If the visual is determiend by a set of 6 textures forming a cube map, this
     variable will be populated.
     */
    std::vector<std::shared_ptr<VROTexture>> _contentsCube;
    
    /*
     Modulates the impact of this visual on the overall material appearance.
     */
    float _intensity;
    
    /*
     Transformation applied to the texture coordinates provided by the geometry object
     the material is attached to.
     */
    VROMatrix4f _contentsTransform;
    
    /*
     Standard texture mapping properties.
     */
    VROWrapMode _wrapS, _wrapT;
    VROFilterMode _minificationFilter, _magnificationFilter, _mipFilter;
    
    VROVector4f _borderColor;
    
};

#endif /* VROMaterialVisual_h */
