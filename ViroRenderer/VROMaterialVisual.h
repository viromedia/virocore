//
//  VROMaterialVisual.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterialVisual_h
#define VROMaterialVisual_h

#include "VROVector3f.h"
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

enum class VROContentType {
    Fixed,
    Texture2D,
    TextureCube
};

class VROMaterialVisual {
    
public:
    
    VROMaterialVisual() :
        _contentType(VROContentType::Fixed),
        _contentsColor({ 1.0, 1.0, 1.0 }),
        _intensity(1.0),
        _wrapS(VROWrapMode::Clamp),
        _wrapT(VROWrapMode::Clamp),
        _minificationFilter(VROFilterMode::Linear),
        _magnificationFilter(VROFilterMode::Linear),
        _mipFilter(VROFilterMode::None)
    {}
    
    void setContents(VROVector3f contents) {
        _contentsColor = contents;
        _contentType = VROContentType::Fixed;
    }
    
    void setContents(std::shared_ptr<VROTexture> texture) {
        _contentsTexture = texture;
        _contentType = VROContentType::Texture2D;
    }
    
    void setContents(std::vector<std::shared_ptr<VROTexture>> cubeTextures) {
        _contentsCube = cubeTextures;
        _contentType = VROContentType::TextureCube;
    }
    
private:
    
    /*
     Indicates the content type for this visual.
     */
    VROContentType _contentType;
    
    /*
     If the visual is determined by a fixed color, _contentsColor is populated.
     */
    VROVector3f _contentsColor;
    
    /*
     If the visual is determiend by a texture, this variable will be populated
     with the texture.
     */
    std::shared_ptr<VROTexture>  _contentsTexture;
    
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
    
    VROVector3f _borderColor;
    
};

#endif /* VROMaterialVisual_h */
