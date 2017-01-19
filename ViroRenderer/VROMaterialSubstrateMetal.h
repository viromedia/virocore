//
//  VROMaterialSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterialSubstrateMetal_h
#define VROMaterialSubstrateMetal_h

#include "VRODefines.h"
#if VRO_METAL

#include "VROMaterial.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <vector>
#include "VROMaterialSubstrate.h"
#include "VROMetalShader.h"

class VROMatrix4f;
class VROVector4f;
class VRODriverMetal;
class VROLight;
class VROConcurrentBuffer;
enum class VROEyeType;

/*
 Metal representation of a VROMaterial. Each VROMaterial defines a vertex
 program and fragment program (by way of the material's lighting model), along
 with the set of uniforms and samplers to bind to said program.
 */
class VROMaterialSubstrateMetal : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateMetal(const VROMaterial &material,
                              VRODriverMetal &driver);
    virtual ~VROMaterialSubstrateMetal();
    
    void bindShader();
    void bindLights(int lightsHash,
                    const std::vector<std::shared_ptr<VROLight>> &lights,
                    const VRORenderContext &context,
                    VRODriver &driver);
    
    /*
     Set the uniforms required to render this material, and return the buffer.
     */
    VROConcurrentBuffer &bindMaterialUniforms(float opacity, VROEyeType eye, int frame);
    
    id <MTLFunction> getVertexProgram() const {
        return _program->getVertexProgram();
    }
    id <MTLFunction> getFragmentProgram() const {
        return _program->getFragmentProgram();
    }
    const std::vector<std::shared_ptr<VROTexture>> &getTextures() const {
        return _textures;
    }
    
    void updateSortKey(VROSortKey &key) const;
    
private:
    
    static std::shared_ptr<VROMetalShader> getPooledShader(std::string vertexShader,
                                                           std::string fragmentShader,
                                                           id <MTLLibrary> library);
    
    const VROMaterial &_material;
    VROLightingModel _lightingModel;
    
    std::shared_ptr<VROMetalShader> _program;
    
    VROConcurrentBuffer *_materialUniformsBuffer;
    VROConcurrentBuffer *_lightingUniformsBuffer;
    
    std::vector<std::shared_ptr<VROTexture>> _textures;
    
    void loadConstantLighting(const VROMaterial &material,
                              id <MTLLibrary> library, id <MTLDevice> device,
                              VRODriverMetal &driver);
    void loadBlinnLighting(const VROMaterial &material,
                           id <MTLLibrary> library, id <MTLDevice> device,
                           VRODriverMetal &driver);
    void loadPhongLighting(const VROMaterial &material,
                           id <MTLLibrary> library, id <MTLDevice> device,
                           VRODriverMetal &driver);
    void loadLambertLighting(const VROMaterial &material,
                             id <MTLLibrary> library, id <MTLDevice> device,
                             VRODriverMetal &driver);
    
    void bindConstantLighting(const std::shared_ptr<VROLight> &light);
    void bindBlinnLighting(const std::shared_ptr<VROLight> &light);
    void bindPhongLighting(const std::shared_ptr<VROLight> &light);
    void bindLambertLighting(const std::shared_ptr<VROLight> &light);
    
    uint32_t hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const;
    
};

#endif
#endif /* VROMaterialSubstrateMetal_h */
