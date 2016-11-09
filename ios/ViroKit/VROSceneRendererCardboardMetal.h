//
//  VROSceneRendererCardboardMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSceneRendererCardboardMetal_h
#define VROSceneRendererCardboardMetal_h

#include "VRODefines.h"
#if VRO_METAL

#import "VROSceneRendererCardboard.h"
#import <memory>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "VRORenderer.h"

class VRORenderTarget;
class VRODriverMetal;
class VROShaderProgram;

/*
 Uses the Metal driver to render a scene to Cardboard. The scene
 is rendered to a Metal texture, which is then blitted via OpenGL
 onto Cardboard's OpenGL context.
 */
class VROSceneRendererCardboardMetal : public VROSceneRendererCardboard {
    
public:
    
    VROSceneRendererCardboardMetal(std::shared_ptr<VRORenderer> renderer);
    virtual ~VROSceneRendererCardboardMetal();
    
    virtual void initRenderer(GVRHeadTransform *headTransform);
    virtual void prepareFrame(VROViewport viewport, VROFieldOfView fov,
                              GVRHeadTransform *headTransform);
    virtual void renderEye(GVREye eye, GVRHeadTransform *headTransform);
    virtual void endFrame();
    
    void setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController);
    void setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController, bool animated);
    void setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController, float seconds,
                            VROTimingFunctionType timingFunctionType);
    
    void setSuspended(bool suspended);
    
private:
    
    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriverMetal> _driver;
    id <MTLCommandBuffer> _commandBuffer;
    std::shared_ptr<VRORenderTarget> _eyeTarget;
    
    /*
     The texture onto which we render both eyes. Eyes are rendered onto the
     MSAA texture and resolved onto _texture.
     */
    id <MTLTexture> _msaaTexture, _texture;
    
    /*
     The GL texture and buffer we use to convert the Metal texture to OpenGL.
     */
    GLuint _textureGL;
    char *_textureBuffer;
    
    float _quadFSVAR[24];
    
    VROShaderProgram *_blitter;
    bool _suspended;
    
    std::shared_ptr<VRORenderTarget> createEyeRenderTarget();
    void writeGLTexture();
    UIImage *writeTextureToImage();
    void drawTexture();
    void drawScreenSpaceVAR();
    void buildFullScreenQuadVAR();
    
};

#endif
#endif /* VROSceneRendererCardboardMetal_h */
