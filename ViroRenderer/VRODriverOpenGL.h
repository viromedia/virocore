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
#include "VROShaderFactory.h"
#include <list>

static const int kResourcePurgeFrameInterval = 120;
static const int kResourcePurgeForceFrameInterval = 1200;

class VRODriverOpenGL : public VRODriver, public std::enable_shared_from_this<VRODriverOpenGL> {

public:
    
    VRODriverOpenGL() :
        _gpuType(VROGPUType::Normal),
        _lastPurgeFrame(0),
        _depthWritingEnabled(true),
        _depthReadingEnabled(true),
        _colorWritingEnabled(true),
        _cullMode(VROCullMode::None),
        _blendMode(VROBlendMode::Alpha) {
        
        _shaderFactory = std::unique_ptr<VROShaderFactory>(new VROShaderFactory());
    }

    void willRenderFrame(const VRORenderContext &context) {
        // Initialize OpenGL state for this frame, matching GL state with CPU state
        // We need to reset state each frame to sync our CPU state with our GPU
        // state, in case a part of the renderer outside our control (e.g. Cardboard,
        // etc.) changes OpenGL state outside of the VRODriver.
        _boundRenderTarget.reset();
        
        _colorWritingEnabled = true;
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        _depthWritingEnabled = true;
        _depthReadingEnabled = true;
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        
        _stencilTestEnabled = true;
        glEnable(GL_STENCIL_TEST);
        
        _cullMode = VROCullMode::None;
        glDisable(GL_CULL_FACE);
        
        _blendMode = VROBlendMode::Alpha;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    void didRenderFrame(const VROFrameTimer &timer, const VRORenderContext &context) {
        if (context.getFrame() - _lastPurgeFrame < kResourcePurgeFrameInterval) {
            return;
        }
        
        // If we haven't had a full purge in awhile, force it, irrespective of time
        // remaining
        bool forcePurge = context.getFrame() - _lastPurgeFrame > kResourcePurgeForceFrameInterval;
        _shaderFactory->purgeUnusedShaders(timer, forcePurge);
        
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
    
    void setStencilTestEnabled(bool enabled) {
        if (_stencilTestEnabled == enabled) {
            return;
        }
        
        _stencilTestEnabled = enabled;
        if (_stencilTestEnabled) {
            glEnable(GL_STENCIL_TEST);
        }
        else {
            glDisable(GL_STENCIL_TEST);
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

    void setBlendingMode(VROBlendMode mode) {
        if (_blendMode == mode) {
            return;
        }

        if (_blendMode != VROBlendMode::None && mode == VROBlendMode::None) {
            glDisable(GL_BLEND);
        }
        else {
            if (_blendMode == VROBlendMode::None && mode != VROBlendMode::None) {
                glEnable(GL_BLEND);
            }
            
            if (mode == VROBlendMode::Alpha) {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else if (mode == VROBlendMode::Add) {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
            else if (mode == VROBlendMode::Multiply){
                glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            }
            else {
                pwarn("Warn: Attempted to use an unsupported blend mode. No blending is applied.");
            }
        }
        _blendMode = mode;
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
    
    void bindShader(std::shared_ptr<VROShaderProgram> program) {
        if (_boundShader == program) {
            return;
        }
        if (_boundShader != nullptr) {
            pglpop();
        }
        
        if (program != nullptr) {
            pglpush("Shader %d [%s]", (int)program->getProgram(), program->getName().c_str());
            program->bind();
        }
        else {
            VROShaderProgram::unbind();
        }
        _boundShader = program;
    }
    
    void unbindShader() {
        if (_boundShader != nullptr) {
            _boundShader.reset();
            pglpop();
            VROShaderProgram::unbind();
        }
    }
    
    void bindRenderTarget(std::shared_ptr<VRORenderTarget> target) {
        std::shared_ptr<VRORenderTarget> boundRenderTarget = _boundRenderTarget.lock();
        /*
         We intentionally leave out a check to see if we're binding the same
         target over again. Including this check causes only the left eye to be
         rendered on Android Axon devices, which is odd: on that device the
         underlying render target must be getting switched external to the
         VRODriver *between* eyes; so when the renderer tries to bind the render
         target for the screen again it's rejected as a no-op.

         To prevent this we just always bind the target we're passed in, and
         don't use our boundRenderTarget to prevent redundant binds.
         */
        if (boundRenderTarget) {
            boundRenderTarget->unbind();
        }
        target->bind();
        _boundRenderTarget = target;
    }
    
    void unbindRenderTarget() {
        std::shared_ptr<VRORenderTarget> boundRenderTarget = _boundRenderTarget.lock();
        if (boundRenderTarget) {
            boundRenderTarget->unbind();
        }
        _boundRenderTarget.reset();
    }
    
    virtual bool isBloomEnabled() {
        return true;
    }

    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return new VROGeometrySubstrateOpenGL(geometry, driver);
    }
    
    VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return new VROMaterialSubstrateOpenGL(material, driver);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type,
                                             VROTextureFormat format,
                                             VROTextureInternalFormat internalFormat, bool sRGB,
                                             VROMipmapMode mipmapMode,
                                             std::vector<std::shared_ptr<VROData>> &data,
                                             int width, int height,
                                             std::vector<uint32_t> mipSizes,
                                             VROWrapMode wrapS, VROWrapMode wrapT,
                                             VROFilterMode minFilter, VROFilterMode magFilter, VROFilterMode mipFilter) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return new VROTextureSubstrateOpenGL(type, format, internalFormat, sRGB, mipmapMode, data,
                                             width, height, mipSizes, wrapS, wrapT, minFilter, magFilter, mipFilter,
                                             driver);
    }
    
    std::shared_ptr<VRORenderTarget> newRenderTarget(VRORenderTargetType type, int numAttachments, int numImages) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VRORenderTarget> target = std::make_shared<VRORenderTargetOpenGL>(type, numAttachments, numImages, driver);
        return target;
    }
    
    std::shared_ptr<VROImagePostProcess> newImagePostProcess(std::shared_ptr<VROShaderProgram> shader) {
        return std::make_shared<VROImagePostProcessOpenGL>(shader);
    }

    void readGPUType() {
        std::string vendor = std::string((char *) glGetString(GL_VENDOR));
        std::string renderer = std::string((char *) glGetString(GL_RENDERER));
        pinfo("GPU vendor [%s], renderer [%s]", vendor.c_str(), renderer.c_str());

        if (VROStringUtil::strcmpinsensitive(vendor, "Qualcomm")) {
            if (renderer.find("302") != std::string::npos ||
                renderer.find("304") != std::string::npos ||
                renderer.find("305") != std::string::npos ||
                renderer.find("306") != std::string::npos ||
                renderer.find("308") != std::string::npos ||
                renderer.find("320") != std::string::npos ||
                renderer.find("330") != std::string::npos) {
                pinfo("   Detected antiquated Qualcomm GPU, rendering will be limited");
                _gpuType = VROGPUType::Adreno330OrOlder;
            }
        }
    }

    VROGPUType getGPUType() {
        return _gpuType;
    }

    void readDisplayFramebuffer() {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_displayFramebuffer);
    }

    virtual std::shared_ptr<VRORenderTarget> getDisplay() {
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
    
    std::unique_ptr<VROShaderFactory> &getShaderFactory() {
        return _shaderFactory;
    }

protected:
    
    /*
     The backbuffer render target.
     */
    std::shared_ptr<VRORenderTarget> _display;

private:

    VROGPUType _gpuType;
    
    /*
     Map of light hashes to corresponding lighting UBOs.
     */
    std::map<int, std::weak_ptr<VROLightingUBO>> _lightingUBOs;
    
    /*
     Creates and caches shaders.
     */
    std::unique_ptr<VROShaderFactory> _shaderFactory;
        
    /*
     The frame during which we last purged resources.
     */
    int _lastPurgeFrame;
    
    /*
     Current context-wide state.
     */
    bool _colorWritingEnabled;
    bool _depthWritingEnabled, _depthReadingEnabled;
    bool _stencilTestEnabled;
    VROCullMode _cullMode;
    VROBlendMode _blendMode;
    
    std::weak_ptr<VRORenderTarget> _boundRenderTarget;
    std::shared_ptr<VROShaderProgram> _boundShader;

    /*
     ID of the backbuffer.
     */
    GLint _displayFramebuffer;
    
};

#endif /* VRODriverOpenGL_h */
