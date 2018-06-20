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
#include <vector>
#include <memory>
#include "VROVector3f.h"

class VRORenderContext;
class VROMaterial;
class VROLight;
enum class VROLightingModel;
enum class VROStereoMode;

enum class VRODiffuseTextureType {
    None,
    YCbCr,
    Normal,
    Cube,
    Text,
};

/*
 Defines the capabilities a shader requires for rendering a given VROMaterial.
 This is derived from a VROMaterial via
 VROShaderCapabilities::deriveMaterialCapabilitiesKey(VROMaterial).
 */
struct VROMaterialShaderCapabilities {

    // Fields used for comparison
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
    bool chromaKeyFiltering;
    int chromaKeyRed, chromaKeyGreen, chromaKeyBlue;
    std::string additionalModifierKeys;
    
    bool operator< (const VROMaterialShaderCapabilities& r) const {
        return std::tie(lightingModel, diffuseTexture, diffuseTextureStereoMode,
                        diffuseEGLModifier, specularTexture, normalTexture, reflectiveTexture,
                        roughnessMap, metalnessMap, aoMap, bloom,
                        receivesShadows,
                        chromaKeyFiltering, chromaKeyRed, chromaKeyGreen, chromaKeyBlue,
                        additionalModifierKeys) <
                std::tie(r.lightingModel, r.diffuseTexture, r.diffuseTextureStereoMode,
                         r.diffuseEGLModifier, r.specularTexture, r.normalTexture, r.reflectiveTexture,
                         r.roughnessMap, r.metalnessMap, r.aoMap, r.bloom,
                         r.receivesShadows,
                         r.chromaKeyFiltering, r.chromaKeyRed, r.chromaKeyGreen, r.chromaKeyBlue,
                         r.additionalModifierKeys);
    }
};

/*
 Defines the capabilities a shader requires for rendering a given lighting
 environment. This is derived from a VRORenderContext and set of Lights via
 VROShaderCapabilities::deriveLightingCapabilitiesKey(VRORenderContext, Lights).
 */
struct VROLightingShaderCapabilities {
    bool shadows;
    bool hdr;
    bool pbr;
    bool diffuseIrradiance;
    bool specularIrradiance;
    
    bool operator< (const VROLightingShaderCapabilities &r) const {
        return std::tie(  shadows,   hdr,   pbr,   diffuseIrradiance,   specularIrradiance)
             < std::tie(r.shadows, r.hdr, r.pbr, r.diffuseIrradiance, r.specularIrradiance);
    }
    bool operator== (const VROLightingShaderCapabilities& r) const {
        return shadows == r.shadows &&
               hdr == r.hdr &&
               pbr == r.pbr &&
               diffuseIrradiance == r.diffuseIrradiance &&
               specularIrradiance == r.specularIrradiance;
    }
    bool operator!= (const VROLightingShaderCapabilities& r) const {
        return shadows != r.shadows ||
               hdr != r.hdr ||
               pbr != r.pbr ||
               diffuseIrradiance != r.diffuseIrradiance ||
               specularIrradiance != r.specularIrradiance;
    }
};

/*
 Defines the capabilities of a VROShaderProgram. These capabilities are a function
 of the VROMaterial being rendered and its lighting environment. Each frame, before
 rendering a VROMaterial, we derive the capabilities it and the current lighting
 environment require in a shader. Once we have the VROShaderCapabilities, we use the
 VROShaderFactory to find a capable VROShaderProgram.
 */
class VROShaderCapabilities {
public:
    VROMaterialShaderCapabilities materialCapabilities;
    VROLightingShaderCapabilities lightingCapabilities;
    
    bool operator< (const VROShaderCapabilities& r) const {
        return std::tie(materialCapabilities, lightingCapabilities) <
               std::tie(r.materialCapabilities, r.lightingCapabilities);
    }
    
    /*
     Derive a key that comprehensively identifies the *capabilities* that the shader
     rendering these lights need. For example, a set of lights that require shadow
     map support will differ from a set of lights that do not.
     */
    static VROLightingShaderCapabilities deriveLightingCapabilitiesKey(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                                       const VRORenderContext &context);
    
    /*
     Derive a key that comprehensively identifies the *capabilities* that the shader
     rendering this material would need. For example, a material that requires
     stereo rendering, or a material that requires textures, will have a key that
     differs from materials that do not.
     */
    static VROMaterialShaderCapabilities deriveMaterialCapabilitiesKey(const VROMaterial &material);
    
};

#endif /* VROShaderCapabilities_hpp */
