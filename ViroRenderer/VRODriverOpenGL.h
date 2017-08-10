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
#include "VRORenderTargetOpenGL.h"
#include "VRODisplayOpenGL.h"
#include "VROShaderProgram.h"
#include "VROLightingUBO.h"
#include "VROShaderModifier.h"
#include "VRORenderContext.h"
#include "VROGeometrySource.h"
#include "VROImagePostProcessOpenGL.h"
#include "VROLight.h"
#include <list>

static const int kResourcePurgeFrameInterval = 120;
static const int kResourcePurgeForceFrameInterval = 1200;

class VRODriverOpenGL : public VRODriver, public std::enable_shared_from_this<VRODriverOpenGL> {

public:
    
    VRODriverOpenGL() :
        _lastPurgeFrame(0),
        _depthWritingEnabled(true),
        _depthReadingEnabled(true),
        _colorWritingEnabled(true),
        _cullMode(VROCullMode::None) {
            
        // Initialize actual OpenGL state to match our CPU state
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
    }

    void willRenderFrame(const VRORenderContext &context) {

    }
    
    void didRenderFrame(const VROFrameTimer &timer, const VRORenderContext &context) {
        if (context.getFrame() - _lastPurgeFrame < kResourcePurgeFrameInterval) {
            return;
        }
        
        // If we haven't had a full purge in awhile, force it, irrespective of time
        // remaining
        bool forcePurge = context.getFrame() - _lastPurgeFrame > kResourcePurgeForceFrameInterval;
        
        // Delete shaders that are only held by the driver, if time provides
        std::map<std::string, std::shared_ptr<VROShaderProgram>>::iterator it = _sharedPrograms.begin();
        while (it != _sharedPrograms.end()) {
            if (!forcePurge && timer.isTimeRemainingInFrame()) {
                return;
            }
            
            if (it->second.unique()) {
                it = _sharedPrograms.erase(it);
            }
            else {
                ++it;
            }
        }
        
        _lastPurgeFrame = context.getFrame();
    }
    
    void setDepthWritingEnabled(bool enabled) {
        if (_depthWritingEnabled == enabled) {
            return;
        }
        
        _depthWritingEnabled = enabled;
        if (enabled) {
            glDepthMask(GL_TRUE);
        }
        else {
            glDepthMask(GL_FALSE);
        }
    }
    
    void setDepthReadingEnabled(bool enabled) {
        if (_depthReadingEnabled == enabled) {
            return;
        }
        
        _depthReadingEnabled = enabled;
        if (_depthReadingEnabled) {
            glDepthFunc(GL_LEQUAL);
        }
        else {
            glDepthFunc(GL_ALWAYS);
        }
    }
    
    void setCullMode(VROCullMode cullMode) {
        if (_cullMode == cullMode) {
            return;
        }
        
        _cullMode = cullMode;
        if (cullMode == VROCullMode::None) {
            glDisable(GL_CULL_FACE);
        }
        else if (cullMode == VROCullMode::Back) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else if (cullMode == VROCullMode::Front) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }
    }
    
    void setColorWritingEnabled(bool enabled) {
        if (_colorWritingEnabled == enabled) {
            return;
        }
        
        _colorWritingEnabled = enabled;
        if (_colorWritingEnabled) {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        else {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        }
    }

    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return new VROGeometrySubstrateOpenGL(geometry, driver);
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
                                             std::vector<uint32_t> mipSizes,
                                             VROWrapMode wrapS, VROWrapMode wrapT,
                                             VROFilterMode minFilter, VROFilterMode magFilter, VROFilterMode mipFilter) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return new VROTextureSubstrateOpenGL(type, format, internalFormat, mipmapMode, data,
                                             width, height, mipSizes, wrapS, wrapT, minFilter, magFilter, mipFilter,
                                             driver);
    }
    
    std::shared_ptr<VRORenderTarget> newRenderTarget(VRORenderTargetType type) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VRORenderTarget> target = std::make_shared<VRORenderTargetOpenGL>(type, driver);
        return target;
    }
    
    std::shared_ptr<VROImagePostProcess> newImagePostProcess(std::shared_ptr<VROShaderProgram> shader) {
        return std::make_shared<VROImagePostProcessOpenGL>(shader);
    }
    
    void readDisplayFramebuffer() {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_displayFramebuffer);
    }

    std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGL>(_displayFramebuffer, driver);
        }
        return _display;
    }
    
    virtual std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() = 0;
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
            return it->second.lock();
        }
        else {
            return {};
        }
    }
    
    std::shared_ptr<VROLightingUBO> createLightingUBO(int lightsHash, const std::vector<std::shared_ptr<VROLight>> &lights) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        
        std::shared_ptr<VROLightingUBO> lightingUBO = std::make_shared<VROLightingUBO>(lightsHash, lights, driver);
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
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();

        std::string modifiersKey = VROShaderModifier::getShaderModifierKey(modifiers);
        std::string name = vertexShader + "_" + fragmentShader + "_" + modifiersKey;
        
        std::map<std::string, std::shared_ptr<VROShaderProgram>>::iterator it = _sharedPrograms.find(name);
        if (it == _sharedPrograms.end()) {
            const std::vector<VROGeometrySourceSemantic> attributes = { VROGeometrySourceSemantic::Texcoord,
                                                                        VROGeometrySourceSemantic::Normal,
                                                                        VROGeometrySourceSemantic::Tangent,
                                                                        VROGeometrySourceSemantic::BoneIndices,
                                                                        VROGeometrySourceSemantic::BoneWeights};
            std::shared_ptr<VROShaderProgram> program = std::make_shared<VROShaderProgram>(vertexShader, fragmentShader,
                                                                                           samplers, modifiers, attributes,
                                                                                           driver);
            _sharedPrograms[name] = program;
            return program;
        }
        else {
            std::shared_ptr<VROShaderProgram> program = it->second;
            
            // Sanity check: all modifiers should be present in the pooled shader
            for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
                passert_msg(program->hasModifier(modifier), "Pooled program %s does not contain modifier %d",
                            name.c_str(), modifier->getShaderModifierId());
            }
            
            return it->second;
        }
    }
    
    /*
     Generate a new binding point for a UBO.
     */
    int generateBindingPoint() {
        if (!_bindingPoints.empty()) {
            int gen = _bindingPoints.back();
            _bindingPoints.pop_back();
            
            return gen;
        }
        else {
            return ++_bindingPointGenerator;
        }
    }

    /*
     Return a binding point that is no longer needed.
     */
    void internBindingPoint(int bindingPoint) {
        _bindingPoints.push_back(bindingPoint);
    }

private:
    
    /*
     List of unused binding points. Binding points bind a UBO to the OpenGL context. 
     They are shader program independent (are shared across programs). These are
     generated incrementally, but returned to this list whenever a UBO is destroyed.
     */
    std::list<int> _bindingPoints;
    int _bindingPointGenerator = 0;
    
    /*
     The backbuffer render target.
     */
    std::shared_ptr<VRORenderTarget> _display;

    /*
     Map of light hashes to corresponding lighting UBOs.
     */
    std::map<int, std::weak_ptr<VROLightingUBO>> _lightingUBOs;
    
    /*
     Shader programs are shared across the system.
     */
    std::map<std::string, std::shared_ptr<VROShaderProgram>> _sharedPrograms;
    
    /*
     The frame during which we last purged resources.
     */
    int _lastPurgeFrame;
    
    /*
     Current context-wide state.
     */
    bool _colorWritingEnabled;
    bool _depthWritingEnabled, _depthReadingEnabled;
    VROCullMode _cullMode;
    
    /*
     ID of the backbuffer.
     */
    GLint _displayFramebuffer;
    
};

#endif /* VRODriverOpenGL_h */
