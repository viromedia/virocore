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

// TODO Delete these imports, convert to Metal, restore glBackup
#include <GLKit/GLKit.h>
#import <OpenGLES/ES2/gl.h>

class Distortion;
class Eye;
class FieldOfView;
class GLStateBackup;
class HeadMountedDisplay;
class Viewport;

class DistortionRenderer {
    
public:
    
    DistortionRenderer();
    ~DistortionRenderer();
    
    void beforeDrawFrame();
    void afterDrawFrame();
    
    void setResolutionScale(float scale);
    
    bool restoreGLStateEnabled();
    void setRestoreGLStateEnabled(bool enabled);
    
    bool chromaticAberrationEnabled();
    void setChromaticAberrationEnabled(bool enabled);
    
    bool vignetteEnabled();
    void setVignetteEnabled(bool enabled);
    
    bool viewportsChanged();
    void updateViewports(VROViewport *leftViewport,
                         VROViewport *rightViewport);
    
    void fovDidChange(VROHeadMountedDisplay *hmd,
                      VROFieldOfView *leftEyeFov,
                      VROFieldOfView *rightEyeFov,
                      float virtualEyeToScreenDistance);
    
    
private:
    
    class DistortionMesh {
    public:
        int _indices;
        int _arrayBufferID;
        int _elementBufferID;
        
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
                       bool vignetteEnabled);
    };
    
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
    
    struct ProgramHolder {
    public:
        ProgramHolder() :
        program(-1),
        positionLocation(-1),
        vignetteLocation(-1),
        redTextureCoordLocation(-1),
        greenTextureCoordLocation(-1),
        blueTextureCoordLocation(-1),
        uTextureCoordScaleLocation(-1),
        uTextureSamplerLocation(-1) {}
        
        GLint program;
        GLint positionLocation;
        GLint vignetteLocation;
        GLint redTextureCoordLocation;
        GLint greenTextureCoordLocation;
        GLint blueTextureCoordLocation;
        GLint uTextureCoordScaleLocation;
        GLint uTextureSamplerLocation;
    };
    
    GLuint _textureID;
    GLuint _renderbufferID;
    GLuint _framebufferID;
    GLenum _textureFormat;
    GLenum _textureType;
    float _resolutionScale;
    bool _restoreGLStateEnabled;
    bool _chromaticAberrationCorrectionEnabled;
    bool _vignetteEnabled;
    DistortionMesh *_leftEyeDistortionMesh;
    DistortionMesh *_rightEyeDistortionMesh;
    //GLStateBackup *_glStateBackup;
    //GLStateBackup *_glStateBackupAberration;
    VROHeadMountedDisplay *_headMountedDisplay;
    EyeViewport *_leftEyeViewport;
    EyeViewport *_rightEyeViewport;
    bool _fovsChanged;
    bool _viewportsChanged;
    bool _textureFormatChanged;
    bool _drawingFrame;
    float _xPxPerTanAngle;
    float _yPxPerTanAngle;
    float _metersPerTanAngle;
    
    ProgramHolder *_programHolder;
    ProgramHolder *_programHolderAberration;
    
    EyeViewport *initViewportForEye(VROFieldOfView *eyeFieldOfView, float xOffsetM);
    
    void setTextureFormat(GLint textureFormat, GLint textureType);
    void updateTextureAndDistortionMesh();
    void undistortTexture(GLint textureID);
    
    DistortionMesh *createDistortionMesh(EyeViewport *eyeViewport,
                                         float textureWidthTanAngle,
                                         float textureHeightTanAngle,
                                         float xEyeOffsetTanAngleScreen,
                                         float yEyeOffsetTanAngleScreen);
    
    void renderDistortionMesh(DistortionMesh *mesh, GLint textureID);
    
    float computeDistortionScale(VRODistortion *distortion,
                                 float screenWidthM,
                                 float interpupillaryDistanceM);
    
    int createTexture(GLint width, GLint height, GLint textureFormat, GLint textureType);
    int setupRenderTextureAndRenderbuffer(int width, int height);
    
    int createProgram(const GLchar *vertexSource,
                      const GLchar *fragmentSource);
    ProgramHolder *createProgramHolder(bool aberrationCorrected);
};

#endif /* VRODistortionRenderer_h */
