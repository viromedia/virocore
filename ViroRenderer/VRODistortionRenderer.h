//
//  VRODistortionRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRODistortionRenderer_h
#define VRODistortionRenderer_h

#include <stdio.h>
#include <MetalKit/MetalKit.h>

class VRODistortion;
class VRODistortionMesh;
class VROFieldOfView;
class VROHeadMountedDisplay;
class VROViewport;
class VRORenderContextMetal;
class VROEye;

/*
 Viewport specified in tangents, i.e. the tangent of the eye's
 field of view angle in each direction.
 */
class VROEyeViewport {
    
public:
    float x;
    float y;
    float width;
    float height;
    float eyeX;
    float eyeY;
    
    VROEyeViewport() :
        x(0), y(0), width(0), height(0), eyeX(0), eyeY(0)
    {}
    VROEyeViewport(const VROFieldOfView &eyeFOV, float xOffset);
    
};

class VRODistortionRenderer {
    
public:
    
    VRODistortionRenderer(VROHeadMountedDisplay &headMountedDisplay);
    ~VRODistortionRenderer();
    
    /*
     Binds the eye texture (the texture to which we will render both eyes) as the 
     render target. Following this call the left and right eye should be rendered.
     */
    id <MTLRenderCommandEncoder> bindEyeRenderTarget(const VRORenderContextMetal &metal);
    
    /*
     Binds the screen as the render target, and renders the left half of the eye 
     texture to the left distortion mesh, and the right half of the eye texture to
     the right distortion mesh.
     */
    void renderEyesToScreen(const VRORenderContextMetal &metal);
    
    void setResolutionScale(float scale) {
        _resolutionScale = scale;
        _viewportsChanged = true;
    }
    
    bool isChromaticAberrationEnabled() {
        return _chromaticAberrationCorrectionEnabled;
    }
    void setChromaticAberrationEnabled(bool enabled) {
        _chromaticAberrationCorrectionEnabled = enabled;
    }
    
    bool isVignetteEnabled() {
        return _vignetteEnabled;
    }
    void setVignetteEnabled(bool enabled) {
        _vignetteEnabled = enabled;
        _fovsChanged = true;
    }
    
    bool viewportsChanged() {
        return _viewportsChanged;
    }
    void updateViewports(VROEye *leftEye, VROEye *rightEye);
    
    void fovDidChange(const VROFieldOfView &leftEyeFov,
                      const VROFieldOfView &rightEyeFov,
                      float virtualEyeToScreenDistance);
    
private:
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLRenderPipelineState> _aberrationPipelineState;
    id <MTLDepthStencilState> _depthState;
    
    /*
     Buffer containing the uniforms needed for the distortion pass.
     */
    id <MTLBuffer> _uniformsBuffer;
    
    /*
     The texture onto which we render both eyes.
     */
    id <MTLTexture> _texture;
    
    float _resolutionScale;
    bool _chromaticAberrationCorrectionEnabled;
    bool _vignetteEnabled;
    
    /*
     The meshes used for barrel distortion for the left and right eye.
     */
    VRODistortionMesh *_leftEyeDistortionMesh;
    VRODistortionMesh *_rightEyeDistortionMesh;
    
    VROHeadMountedDisplay &_headMountedDisplay;
    VROEyeViewport _leftEyeViewport;
    VROEyeViewport _rightEyeViewport;
    
    bool _fovsChanged;
    bool _viewportsChanged;
    bool _drawingFrame;
    
    /*
     The number of pixels per tan(), used to convert an EyeViewport into an actual
     pixel-based viewport.
     */
    float _xPxPerTanAngle;
    float _yPxPerTanAngle;
    float _metersPerTanAngle;
    
    /*
     Create the render encoder used for rendering the eyes to the texture.
     */
    id <MTLRenderCommandEncoder> createEyeRenderEncoder(const VRORenderContextMetal &metal);

    /*
     Update the render-texture and the distortion mesh using hte latest FOV and viewport
     parameters.
     */
    void updateTextureAndDistortionMesh(const VRORenderContextMetal &metal);
    
    /*
     Update the pipeline used to render the distortion pass.
     */
    void updateDistortionPassPipeline(const VRORenderContextMetal &metal);
    
    VRODistortionMesh *createDistortionMesh(const VROEyeViewport &eyeViewport,
                                            float textureWidthTanAngle,
                                            float textureHeightTanAngle,
                                            float xEyeOffsetTanAngleScreen,
                                            float yEyeOffsetTanAngleScreen,
                                            id <MTLDevice> gpu);
    
    void renderDistortionMesh(const VRODistortionMesh &mesh,
                              id <MTLTexture> texture,
                              id <MTLRenderCommandEncoder> renderEncoder);
    
    float computeDistortionScale(const VRODistortion &distortion,
                                 float screenWidthM,
                                 float interpupillaryDistanceM);

};

#endif /* VRODistortionRenderer_h */
