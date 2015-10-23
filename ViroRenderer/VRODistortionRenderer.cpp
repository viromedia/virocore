//
//  VRODistortionRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRODistortionRenderer.h"
#include "VROMath.h"
#include "VROViewport.h"
#include "VROScreen.h"

DistortionRenderer::DistortionRenderer() :
    _textureID(-1),
    _renderbufferID(-1),
    _framebufferID(-1),
    // _originalFramebufferID(-1),
    _textureFormat(GL_RGB),
    _textureType(GL_UNSIGNED_BYTE),
    _resolutionScale(1.0f),
    _restoreGLStateEnabled(true),
    _chromaticAberrationCorrectionEnabled(false),
    _vignetteEnabled(true),
    _leftEyeDistortionMesh(nullptr),
    _rightEyeDistortionMesh(nullptr),
    //_glStateBackup(nullptr),
    //_glStateBackupAberration(nullptr),
    _headMountedDisplay(nullptr),
    _leftEyeViewport(nullptr),
    _rightEyeViewport(nullptr),
    _fovsChanged(false),
    _viewportsChanged(false),
    _textureFormatChanged(false),
    _drawingFrame(false),
    _xPxPerTanAngle(0),
    _yPxPerTanAngle(0),
    _metersPerTanAngle(0),
    _programHolder(nullptr),
    _programHolderAberration(nullptr) {
        
    //_glStateBackup = new GLStateBackup();
    //_glStateBackupAberration = new GLStateBackup();
}

DistortionRenderer::~DistortionRenderer() {
    //if (_glStateBackup != nullptr) { delete _glStateBackup; }
    //if (_glStateBackupAberration != nullptr) { delete _glStateBackupAberration; }
    
    if (_leftEyeDistortionMesh != nullptr) { delete _leftEyeDistortionMesh; }
    if (_rightEyeDistortionMesh != nullptr) { delete _rightEyeDistortionMesh; }
    
    if (_leftEyeViewport != nullptr) { delete _leftEyeViewport; }
    if (_rightEyeViewport != nullptr) { delete _rightEyeViewport; }
    
    if (_programHolder != nullptr) { delete _programHolder; }
    if (_programHolderAberration != nullptr) { delete _programHolderAberration; }
}

void DistortionRenderer::setTextureFormat(GLint textureFormat, GLint textureType)
{
    if (_drawingFrame)
    {
        NSLog(@"Cannot change texture format during rendering.");
    }
    else if (textureFormat != _textureFormat || textureType != _textureType)
    {
        _textureFormat = textureFormat;
        _textureType = textureType;
        _textureFormatChanged = true;
    }
}

void DistortionRenderer::beforeDrawFrame()
{
    _drawingFrame = true;
    
    if (_fovsChanged || _textureFormatChanged)
    {
        updateTextureAndDistortionMesh();
    }
    
    // glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_originalFramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferID);
}

void DistortionRenderer::afterDrawFrame()
{
    // glBindFramebuffer(GL_FRAMEBUFFER, _originalFramebufferID);
    undistortTexture(_textureID);
    _drawingFrame = false;
}

void DistortionRenderer::undistortTexture(GLint textureID)
{
    if (_restoreGLStateEnabled) {
        if (_chromaticAberrationCorrectionEnabled)
        {
        //    _glStateBackupAberration->readFromGL();
        }
        else
        {
         //   _glStateBackup->readFromGL();
        }
    }
    if (_fovsChanged || _textureFormatChanged)
    {
        updateTextureAndDistortionMesh();
    }
    
    std::shared_ptr<VROScreen> screen = _headMountedDisplay->getScreen();
    glViewport(0, 0, screen->getWidth(), screen->getHeight());
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (_chromaticAberrationCorrectionEnabled)
    {
        glUseProgram(_programHolderAberration->program);
    }
    else
    {
        glUseProgram(_programHolder->program);
    }
    
    glEnable(GL_SCISSOR_TEST);
    
    glScissor(0, 0, screen->getWidth() / 2, screen->getHeight());
    
    renderDistortionMesh(_leftEyeDistortionMesh, textureID);
    
    glScissor(screen->getWidth() / 2, 0, screen->getWidth() / 2, screen->getHeight());
    
    renderDistortionMesh(_rightEyeDistortionMesh, textureID);
    
    if (_restoreGLStateEnabled)
    {
        if (_chromaticAberrationCorrectionEnabled)
        {
         //   _glStateBackupAberration->writeToGL();
        }
        else
        {
         //   _glStateBackup->writeToGL();
        }
    }
}

void DistortionRenderer::setResolutionScale(float scale) {
    _resolutionScale = scale;
    _viewportsChanged = true;
}

bool DistortionRenderer::restoreGLStateEnabled() {
    return _restoreGLStateEnabled;;
}

void DistortionRenderer::setRestoreGLStateEnabled(bool enabled) {
    _restoreGLStateEnabled = enabled;
}

bool DistortionRenderer::chromaticAberrationEnabled() {
    return _chromaticAberrationCorrectionEnabled;
}

void DistortionRenderer::setChromaticAberrationEnabled(bool enabled) {
    _chromaticAberrationCorrectionEnabled = enabled;
}

bool DistortionRenderer::vignetteEnabled() {
    return _vignetteEnabled;;
}

void DistortionRenderer::setVignetteEnabled(bool enabled) {
    _vignetteEnabled = enabled;
    _fovsChanged = true;
}

void DistortionRenderer::fovDidChange(VROHeadMountedDisplay *headMountedDisplay,
                                      VROFieldOfView *leftEyeFov,
                                      VROFieldOfView *rightEyeFov,
                                      float virtualEyeToScreenDistance) {
    if (_drawingFrame) {
        NSLog(@"Cannot change FOV while rendering a frame.");
        return;
    }
    
    _headMountedDisplay = headMountedDisplay;
    
    delete (_leftEyeViewport);
    delete (_rightEyeViewport);
    
    _leftEyeViewport = initViewportForEye(leftEyeFov, 0.0f);
    _rightEyeViewport = initViewportForEye(rightEyeFov, _leftEyeViewport->width);
    _metersPerTanAngle = virtualEyeToScreenDistance;
    
    std::shared_ptr<VROScreen> screen = _headMountedDisplay->getScreen();
    _xPxPerTanAngle = screen->getWidth() / ( screen->getWidthInMeters() / _metersPerTanAngle );
    _yPxPerTanAngle = screen->getHeight() / ( screen->getHeightInMeters() / _metersPerTanAngle );
    _fovsChanged = true;
    _viewportsChanged = true;
}

bool DistortionRenderer::viewportsChanged() {
    return _viewportsChanged;
}

void DistortionRenderer::updateViewports(VROViewport *leftViewport, VROViewport *rightViewport) {
    leftViewport->setViewport(round(_leftEyeViewport->x * _xPxPerTanAngle * _resolutionScale),
                              round(_leftEyeViewport->y * _yPxPerTanAngle * _resolutionScale),
                              round(_leftEyeViewport->width * _xPxPerTanAngle * _resolutionScale),
                              round(_leftEyeViewport->height * _yPxPerTanAngle * _resolutionScale));
    rightViewport->setViewport(round(_rightEyeViewport->x * _xPxPerTanAngle * _resolutionScale),
                               round(_rightEyeViewport->y * _yPxPerTanAngle * _resolutionScale),
                               round(_rightEyeViewport->width * _xPxPerTanAngle * _resolutionScale),
                               round(_rightEyeViewport->height * _yPxPerTanAngle * _resolutionScale));
    _viewportsChanged = false;
}

void DistortionRenderer::updateTextureAndDistortionMesh() {
    std::shared_ptr<VROScreen> screen = _headMountedDisplay->getScreen();
    std::shared_ptr<VRODevice> device = _headMountedDisplay->getDevice();
    
    if (_programHolder == nullptr) {
        _programHolder = createProgramHolder(false);
    }
    if (_programHolderAberration == nullptr) {
        _programHolderAberration = createProgramHolder(true);
    }
    
    float textureWidthTanAngle = _leftEyeViewport->width + _rightEyeViewport->width;
    float textureHeightTanAngle = MAX(_leftEyeViewport->height, _rightEyeViewport->height);
    GLint maxTextureSize = 0;
    
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    GLint textureWidthPx = MIN(round(textureWidthTanAngle * _xPxPerTanAngle), maxTextureSize);
    GLint textureHeightPx = MIN(round(textureHeightTanAngle * _yPxPerTanAngle), maxTextureSize);
    
    float xEyeOffsetTanAngleScreen = (screen->getWidthInMeters() / 2.0f - device->getInterLensDistance() / 2.0f) / _metersPerTanAngle;
    float yEyeOffsetTanAngleScreen = (device->getVerticalDistanceToLensCenter() - screen->getBorderSizeInMeters()) / _metersPerTanAngle;
    
    _leftEyeDistortionMesh = createDistortionMesh(_leftEyeViewport,
                                                  textureWidthTanAngle, textureHeightTanAngle,
                                                  xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen);
    
    xEyeOffsetTanAngleScreen = screen->getWidthInMeters() / _metersPerTanAngle - xEyeOffsetTanAngleScreen;
    
    _rightEyeDistortionMesh = createDistortionMesh(_rightEyeViewport,
                                                   textureWidthTanAngle, textureHeightTanAngle,
                                                   xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen);
    setupRenderTextureAndRenderbuffer(textureWidthPx, textureHeightPx);
    
    _fovsChanged = false;
}

DistortionRenderer::EyeViewport *DistortionRenderer::initViewportForEye(VROFieldOfView *eyeFieldOfView, float xOffset) {
    float left = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getLeft()));
    float right = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getRight()));
    float bottom = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getBottom()));
    float top = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getTop()));
    
    EyeViewport *eyeViewport = new EyeViewport();
    eyeViewport->x = xOffset;
    eyeViewport->y = 0.0f;
    eyeViewport->width = (left + right);
    eyeViewport->height = (bottom + top);
    eyeViewport->eyeX = (left + xOffset);
    eyeViewport->eyeY = bottom;
    
    return eyeViewport;
}

DistortionRenderer::DistortionMesh *DistortionRenderer::createDistortionMesh(EyeViewport *eyeViewport,
                                                                             float textureWidthTanAngle,
                                                                             float textureHeightTanAngle,
                                                                             float xEyeOffsetTanAngleScreen,
                                                                             float yEyeOffsetTanAngleScreen) {
    return new DistortionMesh(_headMountedDisplay->getDevice()->getDistortion(),
                              _headMountedDisplay->getDevice()->getDistortion(),
                              _headMountedDisplay->getDevice()->getDistortion(),
                              _headMountedDisplay->getScreen()->getWidthInMeters() / _metersPerTanAngle,
                              _headMountedDisplay->getScreen()->getHeightInMeters() / _metersPerTanAngle,
                              xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen,
                              textureWidthTanAngle, textureHeightTanAngle,
                              eyeViewport->eyeX, eyeViewport->eyeY,
                              eyeViewport->x, eyeViewport->y,
                              eyeViewport->width, eyeViewport->height,
                              _vignetteEnabled);
}

void DistortionRenderer::renderDistortionMesh(DistortionMesh *mesh, GLint textureID)
{
    ProgramHolder *programHolder = nullptr;
    if (_chromaticAberrationCorrectionEnabled)
    {
        programHolder = _programHolderAberration;
    }
    else
    {
        programHolder = _programHolder;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_arrayBufferID);
    glVertexAttribPointer(programHolder->positionLocation, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(programHolder->positionLocation);
    glVertexAttribPointer(programHolder->vignetteLocation, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(programHolder->vignetteLocation);
    glVertexAttribPointer(programHolder->blueTextureCoordLocation, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(programHolder->blueTextureCoordLocation);
    
    if (_chromaticAberrationCorrectionEnabled)
    {
        glVertexAttribPointer(programHolder->redTextureCoordLocation, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(programHolder->redTextureCoordLocation);
        glVertexAttribPointer(programHolder->greenTextureCoordLocation, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(programHolder->greenTextureCoordLocation);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(programHolder->uTextureSamplerLocation, 0);
    glUniform1f(programHolder->uTextureCoordScaleLocation, _resolutionScale);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_elementBufferID);
    glDrawElements(GL_TRIANGLE_STRIP, mesh->_indices, GL_UNSIGNED_SHORT, 0);
}

float DistortionRenderer::computeDistortionScale(VRODistortion *distortion, float screenWidthM, float interpupillaryDistanceM) {
    return distortion->getDistortionFactor((screenWidthM / 2.0f - interpupillaryDistanceM / 2.0f) / (screenWidthM / 4.0f));
}

int DistortionRenderer::createTexture(GLint width, GLint height, GLint textureFormat, GLint textureType)
{
    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0, textureFormat, textureType, nil);
    
    return textureID;
}

int DistortionRenderer::setupRenderTextureAndRenderbuffer(int width, int height)
{
    if (_textureID != -1)
    {
        glDeleteTextures(1, &_textureID);
    }
    if (_renderbufferID != -1)
    {
        glDeleteRenderbuffers(1, &_renderbufferID);
    }
    if (_framebufferID != -1)
    {
        glDeleteFramebuffers(1, &_framebufferID);
    }
    
    _textureID = createTexture(width, height, _textureFormat, _textureType);
    _textureFormatChanged = false;
    
    glGenRenderbuffers(1, &_renderbufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    glGenFramebuffers(1, &_framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferID);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderbufferID);
    
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        [NSException raise:@"DistortionRenderer" format:@"Framebuffer is not complete: %d", status];
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return _framebufferID;
}

int DistortionRenderer::createProgram(const GLchar *vertexSource, const GLchar *fragmentSource) {
    /*
    GLuint vertexShader = 0;
    GLCompileShader(&vertexShader, GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0)
    {
        return 0;
    }
    GLuint pixelShader = 0;
    GLCompileShader(&pixelShader, GL_FRAGMENT_SHADER, fragmentSource);
    if (pixelShader == 0)
    {
        return 0;
    }
    GLuint program = glCreateProgram();
    if (program != 0)
    {
        glAttachShader(program, vertexShader);
        GLCheckForError();
        glAttachShader(program, pixelShader);
        GLCheckForError();
        GLLinkProgram(program);
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLchar message[256];
            glGetProgramInfoLog(program, sizeof(message), 0, &message[0]);
            NSLog(@"Could not link program:\n%s", message);
            glDeleteProgram(program);
            program = 0;
        }
    }
    
    GLCheckForError();
    
    return program;
     */
    return 0;
}

DistortionRenderer::ProgramHolder *DistortionRenderer::createProgramHolder(bool aberrationCorrected)
{
    const GLchar *vertexShader =
    "\
    attribute vec2 aPosition;\n\
    attribute float aVignette;\n\
    attribute vec2 aBlueTextureCoord;\n\
    varying vec2 vTextureCoord;\n\
    varying float vVignette;\n\
    uniform float uTextureCoordScale;\n\
    void main() {\n\
    gl_Position = vec4(aPosition, 0.0, 1.0);\n\
    vTextureCoord = aBlueTextureCoord.xy * uTextureCoordScale;\n\
    vVignette = aVignette;\n\
    }\n";
    
    const GLchar *fragmentShader =
    "\
    precision mediump float;\n\
    varying vec2 vTextureCoord;\n\
    varying float vVignette;\n\
    uniform sampler2D uTextureSampler;\n\
    void main() {\n\
    gl_FragColor = vVignette * texture2D(uTextureSampler, vTextureCoord);\n\
    }\n";
    
    const GLchar *vertexShaderAberration =
    "\
    attribute vec2 aPosition;\n\
    attribute float aVignette;\n\
    attribute vec2 aRedTextureCoord;\n\
    attribute vec2 aGreenTextureCoord;\n\
    attribute vec2 aBlueTextureCoord;\n\
    varying vec2 vRedTextureCoord;\n\
    varying vec2 vBlueTextureCoord;\n\
    varying vec2 vGreenTextureCoord;\n\
    varying float vVignette;\n\
    uniform float uTextureCoordScale;\n\
    void main() {\n\
    gl_Position = vec4(aPosition, 0.0, 1.0);\n\
    vRedTextureCoord = aRedTextureCoord.xy * uTextureCoordScale;\n\
    vGreenTextureCoord = aGreenTextureCoord.xy * uTextureCoordScale;\n\
    vBlueTextureCoord = aBlueTextureCoord.xy * uTextureCoordScale;\n\
    vVignette = aVignette;\n\
    }\n";
    
    const GLchar *fragmentShaderAberration =
    
    "\
    precision mediump float;\n\
    varying vec2 vRedTextureCoord;\n\
    varying vec2 vBlueTextureCoord;\n\
    varying vec2 vGreenTextureCoord;\n\
    varying float vVignette;\n\
    uniform sampler2D uTextureSampler;\n\
    void main() {\n\
    gl_FragColor = vVignette * vec4(texture2D(uTextureSampler, vRedTextureCoord).r,\n\
    texture2D(uTextureSampler, vGreenTextureCoord).g,\n\
    texture2D(uTextureSampler, vBlueTextureCoord).b, 1.0);\n\
    }\n";
    
    ProgramHolder *holder = new ProgramHolder();

    /*
    GLStateBackup *state = nullptr;
    if (aberrationCorrected)
    {
        holder->program = createProgram(vertexShaderAberration, fragmentShaderAberration);
        //state = _glStateBackupAberration;
    }
    else
    {
        holder->program = createProgram(vertexShader, fragmentShader);
        //state = _glStateBackup;
    }
    if (holder->program == 0)
    {
        [NSException raise:@"DistortionRenderer" format:@"Could not create program"];
    }
    
    holder->positionLocation = glGetAttribLocation(holder->program, "aPosition");
    if (holder->positionLocation == -1)
    {
        [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for aPosition"];
    }
    state->addTrackedVertexAttribute(holder->positionLocation);
    
    holder->vignetteLocation = glGetAttribLocation(holder->program, "aVignette");
    if (holder->vignetteLocation == -1)
    {
        [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for aVignette"];
    }
    state->addTrackedVertexAttribute(holder->vignetteLocation);
    
    if (aberrationCorrected)
    {
        holder->redTextureCoordLocation = glGetAttribLocation(holder->program, "aRedTextureCoord");
        if (holder->redTextureCoordLocation == -1)
        {
            [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for aRedTextureCoord"];
        }
        state->addTrackedVertexAttribute(holder->redTextureCoordLocation);
        
        holder->greenTextureCoordLocation = glGetAttribLocation(holder->program, "aGreenTextureCoord");
        if (holder->greenTextureCoordLocation == -1)
        {
            [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for aGreenTextureCoord"];
        }
        state->addTrackedVertexAttribute(holder->greenTextureCoordLocation);
    }
    
    holder->blueTextureCoordLocation = glGetAttribLocation(holder->program, "aBlueTextureCoord");
    GLCheckForError();
    if (holder->blueTextureCoordLocation == -1)
    {
        [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for aBlueTextureCoord"];
    }
    state->addTrackedVertexAttribute(holder->blueTextureCoordLocation);
    
    holder->uTextureCoordScaleLocation = glGetUniformLocation(holder->program, "uTextureCoordScale");
    if (holder->uTextureCoordScaleLocation == -1)
    {
        [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for uTextureCoordScale"];
    }
    
    holder->uTextureSamplerLocation = glGetUniformLocation(holder->program, "uTextureSampler");
    if (holder->uTextureSamplerLocation == -1)
    {
        [NSException raise:@"DistortionRenderer" format:@"Could not get attrib location for uTextureSampler"];
    }
    
    // NSLog(@"ProgramHolder created %p %d", this, holder->program);
    */
    return holder;
}

// DistortionMesh

DistortionRenderer::DistortionMesh::DistortionMesh(VRODistortion *distortionRed,
                                                   VRODistortion *distortionGreen,
                                                   VRODistortion *distortionBlue,
                                                   float screenWidth, float screenHeight,
                                                   float xEyeOffsetScreen, float yEyeOffsetScreen,
                                                   float textureWidth, float textureHeight,
                                                   float xEyeOffsetTexture, float yEyeOffsetTexture,
                                                   float viewportXTexture, float viewportYTexture,
                                                   float viewportWidthTexture, float viewportHeightTexture,
                                                   bool vignetteEnabled) :
_indices(-1), _arrayBufferID(-1), _elementBufferID(-1) {
    GLfloat vertexData[14400];
    
    int vertexOffset = 0;
    
    const int rows = 40;
    const int cols = 40;
    
    const float vignetteSizeTanAngle = 0.05f;
    
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            const float uTextureBlue = col / 39.0f * (viewportWidthTexture / textureWidth) + viewportXTexture / textureWidth;
            const float vTextureBlue = row / 39.0f * (viewportHeightTexture / textureHeight) + viewportYTexture / textureHeight;
            
            const float xTexture = uTextureBlue * textureWidth - xEyeOffsetTexture;
            const float yTexture = vTextureBlue * textureHeight - yEyeOffsetTexture;
            const float rTexture = sqrtf(xTexture * xTexture + yTexture * yTexture);
            
            const float textureToScreenBlue = (rTexture > 0.0f) ? distortionBlue->distortInverse(rTexture) / rTexture : 1.0f;
            
            const float xScreen = xTexture * textureToScreenBlue;
            const float yScreen = yTexture * textureToScreenBlue;
            
            const float uScreen = (xScreen + xEyeOffsetScreen) / screenWidth;
            const float vScreen = (yScreen + yEyeOffsetScreen) / screenHeight;
            const float rScreen = rTexture * textureToScreenBlue;
            
            const float screenToTextureGreen = (rScreen > 0.0f) ? distortionGreen->getDistortionFactor(rScreen) : 1.0f;
            const float uTextureGreen = (xScreen * screenToTextureGreen + xEyeOffsetTexture) / textureWidth;
            const float vTextureGreen = (yScreen * screenToTextureGreen + yEyeOffsetTexture) / textureHeight;
            
            const float screenToTextureRed = (rScreen > 0.0f) ? distortionRed->getDistortionFactor(rScreen) : 1.0f;
            const float uTextureRed = (xScreen * screenToTextureRed + xEyeOffsetTexture) / textureWidth;
            const float vTextureRed = (yScreen * screenToTextureRed + yEyeOffsetTexture) / textureHeight;
            
            const float vignetteSizeTexture = vignetteSizeTanAngle / textureToScreenBlue;
            
            const float dxTexture = xTexture + xEyeOffsetTexture - clamp(xTexture + xEyeOffsetTexture,
                                                                         viewportXTexture + vignetteSizeTexture,
                                                                         viewportXTexture + viewportWidthTexture - vignetteSizeTexture);
            const float dyTexture = yTexture + yEyeOffsetTexture - clamp(yTexture + yEyeOffsetTexture,
                                                                         viewportYTexture + vignetteSizeTexture,
                                                                         viewportYTexture + viewportHeightTexture - vignetteSizeTexture);
            const float drTexture = sqrtf(dxTexture * dxTexture + dyTexture * dyTexture);
            
            float vignette = 1.0f;
            if (vignetteEnabled)
            {
                vignette = 1.0f - clamp(drTexture / vignetteSizeTexture, 0.0f, 1.0f);
            }
            
            vertexData[(vertexOffset + 0)] = 2.0f * uScreen - 1.0f;
            vertexData[(vertexOffset + 1)] = 2.0f * vScreen - 1.0f;
            vertexData[(vertexOffset + 2)] = vignette;
            vertexData[(vertexOffset + 3)] = uTextureRed;
            vertexData[(vertexOffset + 4)] = vTextureRed;
            vertexData[(vertexOffset + 5)] = uTextureGreen;
            vertexData[(vertexOffset + 6)] = vTextureGreen;
            vertexData[(vertexOffset + 7)] = uTextureBlue;
            vertexData[(vertexOffset + 8)] = vTextureBlue;
            
            vertexOffset += 9;
        }
    }
    
    _indices = 3158;
    GLshort indexData[_indices];
    
    int indexOffset = 0;
    vertexOffset = 0;
    for (int row = 0; row < rows-1; row++)
    {
        if (row > 0)
        {
            indexData[indexOffset] = indexData[(indexOffset - 1)];
            indexOffset++;
        }
        for (int col = 0; col < cols; col++)
        {
            if (col > 0)
            {
                if (row % 2 == 0)
                {
                    vertexOffset++;
                }
                else
                {
                    vertexOffset--;
                }
            }
            indexData[(indexOffset++)] = vertexOffset;
            indexData[(indexOffset++)] = (vertexOffset + 40);
        }
        vertexOffset += 40;
    }
    
    GLuint bufferIDs[2] = { 0, 0 };
    glGenBuffers(2, bufferIDs);
    _arrayBufferID = bufferIDs[0];
    _elementBufferID = bufferIDs[1];
    
    glBindBuffer(GL_ARRAY_BUFFER, _arrayBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// EyeViewport

NSString *DistortionRenderer::EyeViewport::toString()
{
    return [NSString stringWithFormat:@"{x:%f y:%f width:%f height:%f eyeX:%f, eyeY:%f}",
            x, y, width, height, eyeX, eyeY];
}