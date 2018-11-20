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

static const bool kEnableStencilCopy = true;
static const int kResourcePurgeFrameInterval = 120;
static const int kResourcePurgeForceFrameInterval = 1200;
static const int kMaxTextureUnits = 32;

class VRODriverOpenGL : public VRODriver, public std::enable_shared_from_this<VRODriverOpenGL> {

public:
    
    VRODriverOpenGL();
    virtual ~VRODriverOpenGL();

    void willRenderFrame(const VRORenderContext &context) {
        // Initialize OpenGL state for this frame, matching GL state with CPU state
        // We need to reset state each frame to sync our CPU state with our GPU
        // state, in case a part of the renderer outside our control (e.g. Cardboard,
        // etc.) changes OpenGL state outside of the VRODriver.
        _boundRenderTarget.reset();
        VROShaderProgram::unbind();
        
        _materialColorWritingMask = VROColorMaskAll;
        _renderTargetColorWritingMask = VROColorMaskAll;
        _aggregateColorWritingMask = VROColorMaskAll;
        GL( glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE) );
        
        _activeTextureUnit = 0;
        GL( glActiveTexture(GL_TEXTURE0) );
        for (int i = 0; i < kMaxTextureUnits; i++) {
            _activeTextures[i].clear();
        }

        _depthWritingEnabled = true;
        _depthReadingEnabled = true;
        GL( glEnable(GL_DEPTH_TEST) );
        GL( glDepthMask(GL_TRUE) );
        GL( glDepthFunc(GL_LEQUAL) );
        
        _stencilTestEnabled = true;
        GL( glEnable(GL_STENCIL_TEST) );
        
        _cullMode = VROCullMode::None;
        GL( glDisable(GL_CULL_FACE) );
        GL( glCullFace(GL_BACK) );
        
        _blendMode = VROBlendMode::Alpha;
        GL( glEnable(GL_BLEND) );
        GL( glBlendEquation(GL_FUNC_ADD) );
        GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

        // Delete all moribund GL objects
        {
            std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
            for (GLuint shader : _moribundShaders) {
                GL( glDeleteShader(shader) );
            }
            _moribundShaders.clear();

            for (GLuint program : _moribundPrograms) {
                GL( glDeleteProgram(program) );
            }
            _moribundPrograms.clear();

            GLsizei count = (GLsizei) _moribundBuffers.size();
            GL( glDeleteBuffers(count, _moribundBuffers.data()) );
            _moribundBuffers.clear();

            count = (GLsizei) _moribundVertexArrays.size();
            GL( glDeleteVertexArrays(count, _moribundVertexArrays.data()) );
            _moribundVertexArrays.clear();

            count = (GLsizei) _moribundFramebuffers.size();
            GL( glDeleteFramebuffers(count, _moribundFramebuffers.data()) );
            _moribundFramebuffers.clear();

            count = (GLsizei) _moribundRenderbuffers.size();
            GL( glDeleteRenderbuffers(count, _moribundRenderbuffers.data()) );
            _moribundRenderbuffers.clear();

            count = (GLsizei) _moribundTextures.size();
            GL( glDeleteTextures(count, _moribundTextures.data()) );
            _moribundTextures.clear();
        }
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
    
    void willRenderEye(const VRORenderContext &context) {
        
    }
    
    void didRenderEye(const VRORenderContext &context) {
        /*
         Unbind the currently bound render target, as it's not clear what the
         platform will do with the bound framebuffer between rendering eyes.
         
         (For example, on Android Axon devices the underlying render target
         does get switched external to the VRODriver between eyes.
         */
        unbindRenderTarget();
    }
    
    void setActiveTextureUnit(int unit) {
        int unitInt = unit - GL_TEXTURE0;
        if (_activeTextureUnit == unitInt) {
            return;
        }
        _activeTextureUnit = unitInt;
        GL( glActiveTexture(unit) );
    }
    
    bool isTextureBound(int unit, int target, int texture) {
        std::map<int, int> &activeTexturesInUnit = _activeTextures[unit];
        
        auto boundTexture = activeTexturesInUnit.find(target);
        return boundTexture != activeTexturesInUnit.end() && boundTexture->second == texture;
    }
    
    void bindTexture(int target, int texture) {
        if (!isTextureBound(_activeTextureUnit, target, texture)) {
            _activeTextures[_activeTextureUnit][target] = texture;
            GL (glBindTexture(target, texture) );
        }
    }
    
    void bindTexture(int unit, int target, int texture) {
        // This method avoids the glActiveTexture call if the texture is already bound
        // to the unit and target
        int unitInt = unit - GL_TEXTURE0;
        if (_activeTextureUnit == unitInt) {
            bindTexture(target, texture);
        } else {
            if (!isTextureBound(unitInt, target, texture)) {
                setActiveTextureUnit(unit);
                bindTexture(target, texture);
            }
        }
    }
    
    void setDepthWritingEnabled(bool enabled) {
        if (_depthWritingEnabled == enabled) {
            return;
        }
        
        _depthWritingEnabled = enabled;
        if (enabled) {
            GL( glDepthMask(GL_TRUE) );
        }
        else {
            GL( glDepthMask(GL_FALSE) );
        }
    }
    
    void setDepthReadingEnabled(bool enabled) {
        if (_depthReadingEnabled == enabled) {
            return;
        }
        
        _depthReadingEnabled = enabled;
        if (_depthReadingEnabled) {
            GL( glDepthFunc(GL_LEQUAL) );
        }
        else {
            GL( glDepthFunc(GL_ALWAYS) );
        }
    }
    
    void setStencilTestEnabled(bool enabled) {
        if (_stencilTestEnabled == enabled) {
            return;
        }
        
        _stencilTestEnabled = enabled;
        if (_stencilTestEnabled) {
            GL( glEnable(GL_STENCIL_TEST) );
        }
        else {
            GL( glDisable(GL_STENCIL_TEST) );
        }
    }

    void setCullMode(VROCullMode cullMode) {
        if (_cullMode == cullMode) {
            return;
        }
        
        _cullMode = cullMode;
        if (cullMode == VROCullMode::None) {
            GL( glDisable(GL_CULL_FACE) );
            GL( glCullFace(GL_BACK) );
        }
        else if (cullMode == VROCullMode::Back) {
            GL( glEnable(GL_CULL_FACE) );
        }
        else if (cullMode == VROCullMode::Front) {
            GL( glEnable(GL_CULL_FACE) );
            GL( glCullFace(GL_FRONT) );
        }
    }

    void setBlendingMode(VROBlendMode mode) {
        if (_blendMode == mode) {
            return;
        }

        if (_blendMode != VROBlendMode::None && mode == VROBlendMode::None) {
            GL( glDisable(GL_BLEND) );
        }
        else {
            if (_blendMode == VROBlendMode::None && mode != VROBlendMode::None) {
                GL( glEnable(GL_BLEND) );
            }
            
            if (mode == VROBlendMode::Alpha) {
                GL( glBlendEquation(GL_FUNC_ADD) );
                GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
            }
            else if (mode == VROBlendMode::Add) {
                GL( glBlendEquation(GL_FUNC_ADD) );
                GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
            }
            else if (mode == VROBlendMode::Multiply) {
                GL( glBlendEquation(GL_FUNC_ADD) );
                GL( glBlendFunc(GL_DST_COLOR, GL_ZERO) );
            }
            else if (mode == VROBlendMode::Subtract) {
                GL( glBlendEquation(GL_FUNC_REVERSE_SUBTRACT) );
                GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
            }
            else if (mode == VROBlendMode::Screen) {
                GL( glBlendEquation(GL_FUNC_ADD) );
                GL( glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR) );
            }
            else if (mode == VROBlendMode::PremultiplyAlpha) {
                GL( glBlendEquation(GL_FUNC_ADD) );
                GL( glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA) );
            }
            else {
                pwarn("Warn: Attempted to use an unsupported blend mode. No blending is applied.");
            }
        }
        _blendMode = mode;
    }
    
    void setRenderTargetColorWritingMask(VROColorMask mask) {
        if (_renderTargetColorWritingMask == mask) {
            return;
        }
        _renderTargetColorWritingMask = mask;
        updateColorMask();
    }
    
    void setMaterialColorWritingMask(VROColorMask mask) {
        if (_materialColorWritingMask == mask) {
            return;
        }
        _materialColorWritingMask = mask;
        updateColorMask();
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
    
    bool bindRenderTarget(std::shared_ptr<VRORenderTarget> target, VRORenderTargetUnbindOp unbindOp) {
        std::shared_ptr<VRORenderTarget> boundRenderTarget = _boundRenderTarget.lock();
        if (boundRenderTarget == target) {
            return false;
        }
        
        if (boundRenderTarget) {
            if (unbindOp == VRORenderTargetUnbindOp::None) {
                target->bind();
            }
            else if (unbindOp == VRORenderTargetUnbindOp::Invalidate) {
                boundRenderTarget->invalidate();
                target->bind();
            }
            else if (unbindOp == VRORenderTargetUnbindOp::CopyStencilAndInvalidate) {
                if (kEnableStencilCopy) {
                    std::shared_ptr<VRODriver> driver = std::dynamic_pointer_cast<VRODriver>(shared_from_this());
                
                    target->bind();
                    boundRenderTarget->blitStencil(target, false, driver);
                    boundRenderTarget->invalidate();
                }
                else {
                    boundRenderTarget->invalidate();
                    target->bind();
                }
            }
        }
        else {
            target->bind();
        }
        _boundRenderTarget = target;
        return true;
    }
    
    void unbindRenderTarget() {
        std::shared_ptr<VRORenderTarget> boundRenderTarget = _boundRenderTarget.lock();
        if (boundRenderTarget) {
            boundRenderTarget->invalidate();
        }
        _boundRenderTarget.reset();
    }
    
    std::shared_ptr<VRORenderTarget> getRenderTarget() {
        return _boundRenderTarget.lock();
    }
    
    virtual bool isBloomSupported() {
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
    
    std::shared_ptr<VRORenderTarget> newRenderTarget(VRORenderTargetType type, int numAttachments, int numImages, bool enableMipmaps,
                                                     bool needsDepthStencil) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VRORenderTarget> target = std::make_shared<VRORenderTargetOpenGL>(type, numAttachments, numImages,
                                                                                          enableMipmaps, needsDepthStencil, driver);
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

        if (VROStringUtil::strcmpinsensitive(vendor, "ARM")) {
            if (renderer.find("Mali") != std::string::npos) {
                pinfo("   Detected Mali GPU, sRGB framebuffer assumed");
                _gpuType = VROGPUType::Mali;
            }

        }
    }

    void setHasSoftwareGammaPass(bool gammaPass) {
        _softwareGammaPass = gammaPass;
    }
    bool hasSoftwareGammaPass() const {
        return _softwareGammaPass;
    }

    VROGPUType getGPUType() {
        return _gpuType;
    }

    void readDisplayFramebuffer() {
        GL( glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_displayFramebuffer) );
    }

    virtual std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGL>(_displayFramebuffer, driver);
        }
        return _display;
    }
    
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

    std::shared_ptr<VROTypefaceCollection> newTypefaceCollection(std::string typefaces, int size, VROFontStyle style,
                                                                 VROFontWeight weight) {
        std::string key = typefaces + "_" + VROStringUtil::toString(size) + "_" +
                                            VROStringUtil::toString((int) style) + "_" +
                                            VROStringUtil::toString((int) weight);
        auto it = _typefaces.find(key);
        if (it == _typefaces.end()) {
            std::shared_ptr<VROTypefaceCollection> typefaceCollection = createTypefaceCollection(typefaces, size, style, weight);
            _typefaces[key] = typefaceCollection;
            return typefaceCollection;
        }
        else {
            std::shared_ptr<VROTypefaceCollection> typefaceCollection = it->second.lock();
            if (typefaceCollection) {
                return typefaceCollection;
            }
            else {
                typefaceCollection = createTypefaceCollection(typefaces, size, style, weight);
                _typefaces[key] = typefaceCollection;
                return typefaceCollection;
            }
        }
    }
    
    std::shared_ptr<VROFrameScheduler> getFrameScheduler() {
        return _scheduler;
    }

    /*
     Queue various GL objects for deletion in a thread-safe manner. This ensures that we only
     delete these objects when the GL context is bound, on the rendering thread.
     */
    void deleteBuffer(GLuint buffer) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundBuffers.push_back(buffer);
    }
    void deleteShader(GLuint shader) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundShaders.push_back(shader);
    }
    void deleteProgram(GLuint program) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundPrograms.push_back(program);
    }
    void deleteFramebuffer(GLuint framebuffer) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundFramebuffers.push_back(framebuffer);
    }
    void deleteRenderbuffer(GLuint renderbuffer) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundRenderbuffers.push_back(renderbuffer);
    }
    void deleteTexture(GLuint texture) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundTextures.push_back(texture);
    }
    void deleteVertexArray(GLuint array) {
        std::lock_guard<std::recursive_mutex> lock(_deletionMutex);
        _moribundVertexArrays.push_back(array);
    }

protected:
    
    /*
     The backbuffer render target.
     */
    std::shared_ptr<VRORenderTarget> _display;

    /*
     Create a typeface collection out of the given comma-separated typeface names. Each typeface will have
     the given size, style, and weight.
     */
    virtual std::shared_ptr<VROTypefaceCollection> createTypefaceCollection(std::string typefaces, int size, VROFontStyle style,
                                                                            VROFontWeight weight) = 0;

    /*
     Get the graphics context used by the underlying platform. Not all platforms will
     implement this.
     */
    virtual void *getGraphicsContext() { return nullptr; }
    
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
     True if a software gamma correction pass is installed.
     */
    bool _softwareGammaPass;
    
    /*
     Active texture unit, and the bound texture on each unit (32 units total).
     Within each unit, there can be multiple textures bound to each target type
     (e.g. GL_TEXTURE_2D, etc.)
     */
    int _activeTextureUnit;
    std::map<int, int> _activeTextures[kMaxTextureUnits];
    
    /*
     Current context-wide state.
     */
    bool _depthWritingEnabled, _depthReadingEnabled;
    VROColorMask _renderTargetColorWritingMask, _materialColorWritingMask, _aggregateColorWritingMask;
    bool _stencilTestEnabled;
    VROCullMode _cullMode;
    VROBlendMode _blendMode;
    
    std::weak_ptr<VRORenderTarget> _boundRenderTarget;
    std::shared_ptr<VROShaderProgram> _boundShader;

    std::recursive_mutex _deletionMutex;
    std::vector<GLuint> _moribundBuffers;
    std::vector<GLuint> _moribundShaders;
    std::vector<GLuint> _moribundPrograms;
    std::vector<GLuint> _moribundFramebuffers;
    std::vector<GLuint> _moribundRenderbuffers;
    std::vector<GLuint> _moribundTextures;
    std::vector<GLuint> _moribundVertexArrays;

    /*
     Caches typeface collections.
     */
    std::map<std::string, std::weak_ptr<VROTypefaceCollection>> _typefaces;
    
    /*
     Responsible for scheduling async tasks on the rendering thread.
     */
    std::shared_ptr<VROFrameScheduler> _scheduler;

    /*
     ID of the backbuffer.
     */
    GLint _displayFramebuffer;
    
    void updateColorMask() {
        // Both the material and render target must support writing to a color for us to
        // enable writing to that color
        VROColorMask mask = (VROColorMask) (_renderTargetColorWritingMask & _materialColorWritingMask);
        if (_aggregateColorWritingMask == mask) {
            return;
        }
        _aggregateColorWritingMask = mask;
        
        GLboolean red, green, blue, alpha;
        red   = (mask & VROColorMaskRed)   == VROColorMaskRed   ? GL_TRUE : GL_FALSE;
        green = (mask & VROColorMaskGreen) == VROColorMaskGreen ? GL_TRUE : GL_FALSE;
        blue  = (mask & VROColorMaskBlue)  == VROColorMaskBlue  ? GL_TRUE : GL_FALSE;
        alpha = (mask & VROColorMaskAlpha) == VROColorMaskAlpha ? GL_TRUE : GL_FALSE;
        GL( glColorMask(red, green, blue, alpha) );
    }
    
};

#endif /* VRODriverOpenGL_h */
