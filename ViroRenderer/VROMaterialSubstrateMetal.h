//
//  VROMaterialSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterialSubstrateMetal_h
#define VROMaterialSubstrateMetal_h

#include "VROMaterial.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <vector>

class VROMatrix4f;
class VROVector4f;
class VRORenderContextMetal;
class VROLight;

/*
 Metal representation of a VROMaterial. Each VROMaterial defines a vertex
 program and fragment program (by way of the material's lighting model), along
 with the set of uniforms and samplers to bind to said program.
 */
class VROMaterialSubstrateMetal {
    
public:
    
    VROMaterialSubstrateMetal(VROMaterial &material,
                              const VRORenderContextMetal &context);
    virtual ~VROMaterialSubstrateMetal();
    
    /*
     Set the uniforms required to render this given material under the
     given light.
     */
    void setLightingUniforms(const std::shared_ptr<VROLight> &light);
    
    id <MTLFunction> getVertexProgram() const {
        return _vertexProgram;
    }
    id <MTLFunction> getFragmentProgram() const {
        return _fragmentProgram;
    }
    id <MTLBuffer> getLightingUniformsBuffer() const {
        return _lightingUniformsBuffer;
    }
    const std::vector<id <MTLTexture>> &getTextures() const {
        return _textures;
    }
    
private:
    
    VROLightingModel _lightingModel;
    
    id <MTLFunction> _vertexProgram;
    id <MTLFunction> _fragmentProgram;
    
    id <MTLBuffer> _lightingUniformsBuffer;
    
    std::vector<id <MTLTexture>> _textures;
    
    void loadConstantLighting(VROMaterial &material,
                              id <MTLLibrary> library, id <MTLDevice> device,
                              const VRORenderContextMetal &context);
    void loadBlinnLighting(VROMaterial &material,
                           id <MTLLibrary> library, id <MTLDevice> device,
                           const VRORenderContextMetal &context);
    void loadPhongLighting(VROMaterial &material,
                           id <MTLLibrary> library, id <MTLDevice> device,
                           const VRORenderContextMetal &context);
    void loadLambertLighting(VROMaterial &material,
                             id <MTLLibrary> library, id <MTLDevice> device,
                             const VRORenderContextMetal &context);
    
    void bindConstantLighting(const std::shared_ptr<VROLight> &light);
    void bindBlinnLighting(const std::shared_ptr<VROLight> &light);
    void bindPhongLighting(const std::shared_ptr<VROLight> &light);
    void bindLambertLighting(const std::shared_ptr<VROLight> &light);
    
};

#endif /* VROMaterialSubstrateMetal_h */
