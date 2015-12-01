//
//  VROMaterialSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMaterialSubstrateMetal_h
#define VROMaterialSubstrateMetal_h

#include "VRORenderContextMetal.h"
#include "VROMaterial.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <vector>

class VROVector4f;

/*
 Metal representation of a VROMaterial. Each VROMaterial defines a vertex
 program and fragment program (by way of the material's lighting model), along
 with the set of uniforms and samplers to bind to said program.
 */
class VROMaterialSubstrateMetal {
    
public:
    
    VROMaterialSubstrateMetal(const VRORenderContextMetal &context,
                              std::shared_ptr<VROMaterial> material);
    virtual ~VROMaterialSubstrateMetal();
    
private:
    
    id <MTLFunction> _vertexProgram;
    id <MTLFunction> _fragmentProgram;
    
    id <MTLBuffer>   _uniformsBuffer;
    std::vector<id <MTLTexture>> _textures;
    
    void loadConstantLighting(std::shared_ptr<VROMaterial> material,
                              id <MTLLibrary> library, id <MTLDevice> device);
    void loadBlinnLighting(std::shared_ptr<VROMaterial> material,
                           id <MTLLibrary> library, id <MTLDevice> device);
    void loadPhongLighting(std::shared_ptr<VROMaterial> material,
                           id <MTLLibrary> library, id <MTLDevice> device);
    void loadLambertLighting(std::shared_ptr<VROMaterial> material,
                             id <MTLLibrary> library, id <MTLDevice> device);
    
    
    
};

#endif /* VROMaterialSubstrateMetal_h */
