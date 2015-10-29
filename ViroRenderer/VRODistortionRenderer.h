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
#include "VROHeadMountedDisplay.h"
#include "VROViewport.h"
#include "VROFieldOfView.h"
#include "VRODistortion.h"

class Distortion;
class Eye;
class FieldOfView;
class HeadMountedDisplay;
class Viewport;
class VRORenderContextMetal;

class VRODistortionRenderer {
    
public:
    
    VRODistortionRenderer();
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
    void updateViewports(VROViewport *leftViewport,
                         VROViewport *rightViewport);
    
    void fovDidChange(VROHeadMountedDisplay *hmd,
                      VROFieldOfView *leftEyeFov,
                      VROFieldOfView *rightEyeFov,
                      float virtualEyeToScreenDistance);
    
    
private:
    
    class DistortionMesh {
    public:

        id <MTLBuffer> _vertexBuffer;
        id <MTLBuffer> _indexBuffer;
        
        DistortionMesh();
        DistortionMesh(VRODistortion *distortionRed,
                       VRODistortion *distortionGreen,
                       VRODistortion *distortionBlue,
                       float screenWidth, float screenHeight,
                       float xEyeOffsetScreen, float yEyeOffsetScreen,
                       float textureWidth, float textureHeight,
                       float xEyeOffsetTexture, float yEyeOffsetTexture,
                       float viewportXTexture, float viewportYTexture,
                       float viewportWidthTexture,
                       float viewportHeightTexture,
                       bool vignetteEnabled,
                       id <MTLDevice> gpu);
    };
    
    /*
     Viewport specified in tangents, i.e. the tangent of the eye's 
     field of view angle in each direction.
     */
    struct EyeViewport {
    public:
        float x;
        float y;
        float width;
        float height;
        float eyeX;
        float eyeY;
        
        NSString *toString();
    };
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLRenderPipelineState> _aberrationPipelineState;
    id <MTLDepthStencilState> _depthState;
    
    id <MTLBuffer> _uniformsBuffer;
    
    /*
     The texture to which we render both eyes.
     */
    id <MTLTexture> _texture;
    
    float _resolutionScale;
    bool _chromaticAberrationCorrectionEnabled;
    bool _vignetteEnabled;
    DistortionMesh *_leftEyeDistortionMesh;
    DistortionMesh *_rightEyeDistortionMesh;
    
    VROHeadMountedDisplay *_headMountedDisplay;
    EyeViewport *_leftEyeViewport;
    EyeViewport *_rightEyeViewport;
    
    bool _fovsChanged;
    bool _viewportsChanged;
    bool _drawingFrame;
    
    /*
     Measurement parameters.
     */
    float _xPxPerTanAngle;
    float _yPxPerTanAngle;
    float _metersPerTanAngle;
    
    EyeViewport *initViewportForEye(VROFieldOfView *eyeFieldOfView, float xOffsetM);
    
    void updateTextureAndDistortionMesh(const VRORenderContextMetal &metal);
    id <MTLRenderCommandEncoder> createEyeRenderEncoder(const VRORenderContextMetal &metal);
    void updateDistortionMeshPipeline(const VRORenderContextMetal &metal);
    
    DistortionMesh *createDistortionMesh(EyeViewport *eyeViewport,
                                         float textureWidthTanAngle,
                                         float textureHeightTanAngle,
                                         float xEyeOffsetTanAngleScreen,
                                         float yEyeOffsetTanAngleScreen,
                                         id <MTLDevice> gpu);
    
    void renderDistortionMesh(DistortionMesh *mesh, id <MTLTexture> texture, id <MTLRenderCommandEncoder> renderEncoder);
    
    float computeDistortionScale(VRODistortion *distortion,
                                 float screenWidthM,
                                 float interpupillaryDistanceM);
    
    void setupRenderTexture(int width, int height, id <MTLDevice> gpu);

};

#endif /* VRODistortionRenderer_h */
