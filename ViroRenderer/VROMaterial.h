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

enum class VROCullMode {
    Back,
    Front,
    None
};

enum class VROBlendMode {
    Alpha,
    Add,
    Subtract,
    Multiply,
    Screen,
    Replace
};

enum class VROTransparencyMode {
    AOne,
    RGBZero
};

enum class VROLightingModel {
    Phong,
    Blinn,
    Lambert,
    Constant
};

/*
 Manages the lighting and shading attributes associated with the surface of a geometry that
 define its appearance when rendered. When you create a material, you define a collection of
 visual attributes and their options, which you can then reuse for multiple geometries 
 in a scene.
 */
class VROMaterial {
    
public:
    
    VROMaterial();
    virtual ~VROMaterial();
    
private:
    
    /*
     The visual properties associated with the material. These are optional.
     */
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
    
    /*
     User-provided name of the material.
     */
    std::string _name;
    
    /*
     The sharpness of specular highlights.
     */
    float _shininess;
    
    /*
     Factor affecting material reflectivity.
     */
    float _fresnelExponent;
    
    /*
     Uniform transparency of the material.
     */
    float _transparency;
    
    /*
     The mode used to calculate transparency.
     */
    VROTransparencyMode _transparencyMode;
    
    /*
     The lighting model to use to compute the interaction between
     the lights in the scene and this material's visual properties.
     */
    VROLightingModel _lightingModel;
    
    /*
     True means use per-pixel lighting, false means use per-vertex lighting.
     */
    bool _litPerPixel;
    
    /*
     Set to cull back faces, front faces, or none.
     */
    VROCullMode _cullMode;
    
    /*
     Determines how pixel colors rendered using this material blend with 
     pixel colors already in the render target.
     */
    VROBlendMode _blendMode;
    
    /*
     Depth write and read settings.
     */
    bool _writesToDepthBuffer, _readsFromDepthBuffer;
    
};

#endif /* VROMaterial_h */
