//
//  VROShaderCapabilities.hpp
//  ViroKit
//
//  Created by Raj Advani on 8/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROShaderCapabilities_hpp
#define VROShaderCapabilities_hpp

#include <string>

enum class VRODiffuseTextureType {
    None,
    YCbCr,
    Normal,
    Cube
};

struct VROMaterialShaderCapabilities {
    VROLightingModel lightingModel;
    VRODiffuseTextureType diffuseTexture;
    VROStereoMode diffuseTextureStereoMode;
    bool diffuseEGLModifier;
    bool specularTexture;
    bool normalTexture;
    bool reflectiveTexture;
    bool roughnessMap, metalnessMap, aoMap;
    bool bloom;
    bool receivesShadows;
    std::string additionalModifierKeys;
    
    bool operator< (const VROMaterialShaderCapabilities& r) const {
        return std::tie(lightingModel, diffuseTexture, diffuseTextureStereoMode,
                        diffuseEGLModifier, specularTexture, normalTexture, reflectiveTexture, bloom,
                        receivesShadows, additionalModifierKeys) <
                std::tie(r.lightingModel, r.diffuseTexture, r.diffuseTextureStereoMode,
                         r.diffuseEGLModifier, r.specularTexture, r.normalTexture, r.reflectiveTexture, r.bloom,
                         r.receivesShadows, r.additionalModifierKeys);
    }
};

struct VROLightingShaderCapabilities {
    bool shadows;
    
    bool operator< (const VROLightingShaderCapabilities& r) const {
        return std::tie(shadows) < std::tie(r.shadows);
    }
    bool operator== (const VROLightingShaderCapabilities& r) const {
        return shadows == r.shadows;
    }
    bool operator!= (const VROLightingShaderCapabilities& r) const {
        return shadows != r.shadows;
    }
};

struct VROShaderCapabilities {
    VROMaterialShaderCapabilities materialCapabilities;
    VROLightingShaderCapabilities lightingCapabilities;
    
    bool operator< (const VROShaderCapabilities& r) const {
        return std::tie(materialCapabilities, lightingCapabilities) <
        std::tie(r.materialCapabilities, r.lightingCapabilities);
    }
};

#endif /* VROShaderCapabilities_hpp */
