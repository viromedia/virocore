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

class VROMaterialVisual {
    
public:
    
    void setContents(VROVector3f contents) {
        _contentsColor = contents;
    }
    void setContents(std::shared_ptr<VROTexture> texture) {
        _contentsTexture = texture;
    }
    void setContents(std::vector<std::shared_ptr<VROTexture>> cubeTextures) {
        _contentsCube = cubeTextures;
    }
    
private:
    
    /*
     Only one of these can be populated.
     */
    VROVector3f _contentsColor;
    std::shared_ptr<VROTexture>  _contentsTexture;
    std::vector<std::shared_ptr<VROTexture>> _contentsCube;
    
    float _intensity;
    
    VROMatrix4f _contentsTransform;
    
    VROWrapMode _wrapS, _wrapT;
    VROFilterMode _minificationFilter, _magnificationFilter, _mipFilter;
    
    float _maxAnisotropy;
    VROVector3f _borderColor;
    
};

#endif /* VROMaterialVisual_h */
