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
#include "VRODefines.h"
#include "VROStringUtil.h"
#include "VROGeometrySubstrateOpenGL.h"
#include "VROMaterialSubstrateOpenGL.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROShaderProgram.h"
#include "VROLightingUBO.h"
#include "VROShaderModifier.h"
#include "VROGeometrySource.h"
#include "VROLight.h"

class VRODriverOpenGL : public VRODriver {

public:

    void onFrame(const VRORenderContext &context) {

    }
    
    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) {
        return new VROGeometrySubstrateOpenGL(geometry, *this);
    }
    
    VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) {
        return new VROMaterialSubstrateOpenGL(material, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type,
                                             VROTextureFormat format,
                                             VROTextureInternalFormat internalFormat,
                                             VROMipmapMode mipmapMode,
                                             std::vector<std::shared_ptr<VROData>> &data,
                                             int width, int height,
                                             std::vector<uint32_t> mipSizes) {
        return new VROTextureSubstrateOpenGL(type, format, internalFormat, mipmapMode, data,
                                             width, height, mipSizes, *this);
    }
    
    virtual VROVideoTextureCache *newVideoTextureCache() = 0;
    virtual std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) = 0;
    virtual std::shared_ptr<VROSound> newSound(std::string fileName, VROSoundType type, bool local) = 0;
    virtual std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) = 0;
    virtual std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string fileName, bool isLocal) = 0;
    virtual std::shared_ptr<VROTypeface> newTypeface(std::string typeface, int size) = 0;
    virtual void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                              std::string ceilingMaterial, std::string floorMaterial) = 0;
    
    std::shared_ptr<VROLightingUBO> getLightingUBO(int lightsHash) {
        auto it = _lightingUBOs.find(lightsHash);
        if (it != _lightingUBOs.end()) {
            std::shared_ptr<VROLightingUBO> light = it->second.lock();
            if (light) {
                return light;
            }
            else {
                return {};
            }
        }
        else {
            return {};
        }
    }
    
    std::shared_ptr<VROLightingUBO> createLightingUBO(int lightsHash, const std::vector<std::shared_ptr<VROLight>> &lights) {
        std::shared_ptr<VROLightingUBO> lightingUBO = std::make_shared<VROLightingUBO>(lightsHash, lights);
        _lightingUBOs[lightsHash] = lightingUBO;
        
        for (const std::shared_ptr<VROLight> &light : lights) {
            light->addUBO(lightingUBO);
        }
        return lightingUBO;
    }
    
    std::shared_ptr<VROShaderProgram> getPooledShader(std::string vertexShader,
                                                      std::string fragmentShader,
                                                      const std::vector<std::string> &samplers,
                                                      const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers) {
        int modifiersHash = VROShaderModifier::hashShaderModifiers(modifiers);
        std::string name = vertexShader + "_" + fragmentShader + "_" + VROStringUtil::toString(modifiersHash);
        
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

    /*
     Map of light hashes to corresponding lighting UBOs.
     */
    std::map<int, std::weak_ptr<VROLightingUBO>> _lightingUBOs;
    
    /*
     Shader programs are shared across the system.
     */
    std::map<std::string, std::shared_ptr<VROShaderProgram>> _sharedPrograms;
    
};

#endif /* VRODriverOpenGL_h */
