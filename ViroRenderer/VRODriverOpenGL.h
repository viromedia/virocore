//
//  VRODriverOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGL_h
#define VRODriverOpenGL_h

#include "VRODriver.h"
#include "VROGeometrySubstrateOpenGL.h"
#include "VROMaterialSubstrateOpenGL.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROShaderProgram.h"
#include "VROLightingUBO.h"
#include "VROShaderModifier.h"
#include "VROGeometrySource.h"

class VRODriverOpenGL : public VRODriver {
    
public:
    
    VRODriverOpenGL(EAGLContext *eaglContext) :
        _eaglContext(eaglContext) {

    }
    
    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) {
        return new VROGeometrySubstrateOpenGL(geometry, *this);
    }
    
    VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) {
        return new VROMaterialSubstrateOpenGL(material, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<std::shared_ptr<VROImage>> &images) {
        return new VROTextureSubstrateOpenGL(type, images, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                             int width, int height) {
        return new VROTextureSubstrateOpenGL(type, format, data, width, height, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(int width, int height, CGContextRef bitmapContext) {
        return new VROTextureSubstrateOpenGL(width, height, bitmapContext, *this);
    }
    
    VROVideoTextureCache *newVideoTextureCache() {
        return new VROVideoTextureCacheOpenGL(_eaglContext);
    }
    
    std::shared_ptr<VROLightingUBO> getLightingUBO(int lightsHash) {
        auto it = _lightingUBOs.find(lightsHash);
        if (it != _lightingUBOs.end()) {
            return it->second;
        }
        else {
            return {};
        }
    }
    
    std::shared_ptr<VROLightingUBO> createLightingUBO(int lightsHash) {
        std::shared_ptr<VROLightingUBO> lightingUBO = std::make_shared<VROLightingUBO>();
        _lightingUBOs[lightsHash] = lightingUBO;
        
        return lightingUBO;
    }
    
    std::shared_ptr<VROShaderProgram> getPooledShader(std::string vertexShader,
                                                      std::string fragmentShader,
                                                      const std::vector<std::string> &samplers,
                                                      const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers) {
        int modifiersHash = VROShaderModifier::hashShaderModifiers(modifiers);
        std::string name = vertexShader + "_" + fragmentShader + "_" + std::to_string(modifiersHash);
        
        std::map<std::string, std::shared_ptr<VROShaderProgram>>::iterator it = _sharedPrograms.find(name);
        if (it == _sharedPrograms.end()) {
            const std::vector<VROGeometrySourceSemantic> attributes = { VROGeometrySourceSemantic::Texcoord,
                VROGeometrySourceSemantic::Normal };
            std::shared_ptr<VROShaderProgram> program = std::make_shared<VROShaderProgram>(vertexShader, fragmentShader,
                                                                                           samplers, modifiers, attributes);
            _sharedPrograms[name] = program;
            return program;
        }
        else {
            return it->second;
        }
    }
    
private:
    
    EAGLContext *_eaglContext;

    /*
     Map of light hashes to corresponding lighting UBOs.
     */
    std::map<int, std::shared_ptr<VROLightingUBO>> _lightingUBOs;
    
    /*
     Shader programs are shared across the system.
     */
    std::map<std::string, std::shared_ptr<VROShaderProgram>> _sharedPrograms;
    
};

#endif /* VRODriverOpenGL_h */
