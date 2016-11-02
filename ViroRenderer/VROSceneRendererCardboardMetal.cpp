//
//  VROSceneRendererCardboardMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROSceneRendererCardboardMetal.h"
#if VRO_METAL

#import <GLKit/GLKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "VRODriverMetal.h"
#import "VROShaderProgram.h"
#import "VRORenderTarget.h"
#import "VROViewport.h"
#import "VROEye.h"
#import "VROConvert.h"

static const float kSampleCount = 4;
static const MTLPixelFormat kResolvePixelFormat = MTLPixelFormatRGBA8Unorm;

VROSceneRendererCardboardMetal::VROSceneRendererCardboardMetal(std::shared_ptr<VRORenderer> renderer) :
    _frame(0),
    _renderer(renderer),
    _suspended(true) {
    
    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    _driver = std::make_shared<VRODriverMetal>(device);
}

VROSceneRendererCardboardMetal::~VROSceneRendererCardboardMetal() {
    free (_textureBuffer);
    delete (_blitter);
}

void VROSceneRendererCardboardMetal::initRenderer(GVRHeadTransform *headTransform) {
    id <MTLDevice> gpu = _driver->getDevice();
    
    /*
     Compute the size of the required eye render texture (the texture to which we render
     BOTH eyes), and create it.
     */
    int maxTextureSize = 2048; // TODO query GPU to find this
    
    CGRect leftViewport  = [headTransform viewportForEye:kGVRLeftEye];
    CGRect rightViewport = [headTransform viewportForEye:kGVRRightEye];
    
    int textureWidthPx  = MIN(leftViewport.size.width + rightViewport.size.width,
                              maxTextureSize);
    int textureHeightPx = MIN(round(MAX(leftViewport.size.height, rightViewport.size.height)),
                              maxTextureSize);
    
    MTLTextureDescriptor *msaaDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:kResolvePixelFormat
                                                                                        width:textureWidthPx
                                                                                       height:textureHeightPx
                                                                                    mipmapped:NO];
    msaaDesc.textureType = MTLTextureType2DMultisample;
    msaaDesc.sampleCount = kSampleCount;
    
    _msaaTexture = [gpu newTextureWithDescriptor:msaaDesc];
    
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:kResolvePixelFormat
                                                                                          width:textureWidthPx
                                                                                         height:textureHeightPx
                                                                                      mipmapped:NO];
    _texture = [gpu newTextureWithDescriptor:descriptor];
    
    buildFullScreenQuadVAR();
    glGenTextures(1, &_textureGL);
    
    uint32_t bytesPerPixel = 4;
    uint32_t bytesPerRow = bytesPerPixel * (uint32_t)[_texture width];
    _textureBuffer = (char *) malloc(bytesPerRow * [_texture height]);
    
    _blitter = new VROShaderProgram("blit_vsh", "blit_fsh",  { "map" }, {}, {});
    _blitter->hydrate();
}

void VROSceneRendererCardboardMetal::setSceneController(VROSceneController *sceneController) {
    _renderer->setSceneController(sceneController, *_driver);
}

void VROSceneRendererCardboardMetal::setSceneController(VROSceneController *sceneController, bool animated) {
    _renderer->setSceneController(sceneController, animated, *_driver);
}

void VROSceneRendererCardboardMetal::setSceneController(VROSceneController *sceneController, float seconds,
                                                        VROTimingFunctionType timingFunctionType) {
    
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, *_driver);
}

void VROSceneRendererCardboardMetal::prepareFrame(GVRHeadTransform *headTransform) {
    VRODriverMetal *driver = (VRODriverMetal *)_driver.get();
    
    _commandBuffer = [driver->getCommandQueue() commandBuffer];
    _commandBuffer.label = @"CommandBuffer";
    
    _eyeTarget = createEyeRenderTarget();
    _driver->setRenderTarget(_eyeTarget);
    
    VROMatrix4f headRotation = VROConvert::toMatrix4f([headTransform headPoseInStartSpace]);
    _renderer->prepareFrame(_frame, headRotation, *_driver.get());
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

void VROSceneRendererCardboardMetal::renderEye(GVREye eye, GVRHeadTransform *headTransform) {
    if (_suspended) {
        return;
    }
    
    CGRect rect = [headTransform viewportForEye:eye];
    VROViewport viewport(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    
    VROMatrix4f eyeMatrix = VROConvert::toMatrix4f([headTransform eyeFromHeadMatrix:eye]);
    VROMatrix4f projectionMatrix = VROConvert::toMatrix4f([headTransform projectionMatrixForEye:eye near:kZNear far:kZFar]);
    
    id <MTLRenderCommandEncoder> renderEncoder = _eyeTarget->getRenderEncoder();
    [renderEncoder setViewport:viewport.toMetalViewport()];
    [renderEncoder setScissorRect:viewport.toMetalScissor()];
    
    VROEyeType eyeType = (eye == kGVRLeftEye ? VROEyeType::Left : VROEyeType::Right);
    _renderer->renderEye(eyeType, eyeMatrix, projectionMatrix, *_driver.get());
}

void VROSceneRendererCardboardMetal::endFrame() {
    _renderer->endFrame(*_driver.get());
    
    id <MTLRenderCommandEncoder> renderEncoder = _eyeTarget->getRenderEncoder();
    [renderEncoder endEncoding];
    
    writeGLTexture();
    
    /*
     We write the entire OpenGL texture to the screen, so set the viewport
     and scissor to the full width and height.
     */
    glViewport(0, 0, (int)[_texture width], (int)[_texture height]);
    glScissor(0, 0, (int)[_texture width], (int)[_texture height]);
    
    drawTexture();
    
    [_commandBuffer commit];
    [_driver->getCommandQueue() insertDebugCaptureBoundary];
    
    ++_frame;
}

std::shared_ptr<VRORenderTarget> VROSceneRendererCardboardMetal::createEyeRenderTarget() {
    MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = _msaaTexture;
    renderPassDesc.colorAttachments[0].resolveTexture = _texture;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
    eyeRenderEncoder.label = @"EyeRenderEncoder";
    
    [eyeRenderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [eyeRenderEncoder setCullMode:MTLCullModeBack];
    
    return std::make_shared<VRORenderTarget>(eyeRenderEncoder, _texture.pixelFormat, MTLPixelFormatInvalid, _msaaTexture.sampleCount);
}

void VROSceneRendererCardboardMetal::writeGLTexture() {
    glBindTexture(GL_TEXTURE_2D, _textureGL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    MTLRegion region = MTLRegionMake2D(0, 0, [_texture width], [_texture height]);
    uint32_t bytesPerPixel = 4;
    uint64_t bytesPerRow = bytesPerPixel * [_texture width];
    
    [_texture getBytes:_textureBuffer bytesPerRow:bytesPerRow fromRegion:region mipmapLevel:0];
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)[_texture width], (int)[_texture height], 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, _textureBuffer);
}

UIImage *VROSceneRendererCardboardMetal::writeTextureToImage() {
    MTLRegion region = MTLRegionMake2D(0, 0, [_texture width], [_texture height]);
    uint32_t bytesPerPixel = 4;
    uint64_t bytesPerRow = bytesPerPixel * [_texture width];
    
    [_texture getBytes:_textureBuffer bytesPerRow:bytesPerRow fromRegion:region mipmapLevel:0];
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, _textureBuffer, [_texture width] * [_texture height] * 4, NULL);
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef imageRef = CGImageCreate([_texture width] , [_texture height], 8, 32, 4 * [_texture width],
                                        colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    return [UIImage imageWithCGImage:imageRef];
}

void VROSceneRendererCardboardMetal::drawTexture() {
    glDisable(GL_DEPTH_TEST);
    
    glBindTexture(GL_TEXTURE_2D, _textureGL);
    drawScreenSpaceVAR();
    
    glEnable(GL_DEPTH_TEST);
}

void VROSceneRendererCardboardMetal::drawScreenSpaceVAR() {
    glDisable(GL_CULL_FACE);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    _blitter->bind();
    
    glEnableVertexAttribArray((int) VROGeometrySourceSemantic::Vertex);
    glEnableVertexAttribArray((int) VROGeometrySourceSemantic::Texcoord);
    
    glVertexAttribPointer((int) VROGeometrySourceSemantic::Vertex, 2, GL_FLOAT, 0, 16, _quadFSVAR);
    glVertexAttribPointer((int) VROGeometrySourceSemantic::Texcoord, 2, GL_FLOAT, 0, 16, ((char *) _quadFSVAR + 2 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void VROSceneRendererCardboardMetal::buildFullScreenQuadVAR() {
    float qstartV = 1.0f;
    float qendV = 0.0f;
    float qendU = 1.0f;
    
    float qleft = -1;
    float qright = 1;
    float qbottom = -1;
    float qtop = 1;
    
    //BL
    _quadFSVAR[0] = qleft;
    _quadFSVAR[1] = qbottom;
    _quadFSVAR[2] = 0;
    _quadFSVAR[3] = qstartV;
    
    //BR
    _quadFSVAR[4] = qright;
    _quadFSVAR[5] = qbottom;
    _quadFSVAR[6] = qendU;
    _quadFSVAR[7] = qstartV;
    
    //TL
    _quadFSVAR[8] = qleft;
    _quadFSVAR[9] = qtop;
    _quadFSVAR[10] = 0;
    _quadFSVAR[11] = qendV;
    
    //TR
    _quadFSVAR[12] = qright;
    _quadFSVAR[13] = qtop;
    _quadFSVAR[14] = qendU;
    _quadFSVAR[15] = qendV;
    
    //TL
    _quadFSVAR[16] = qleft;
    _quadFSVAR[17] = qtop;
    _quadFSVAR[18] = 0;
    _quadFSVAR[19] = qendV;
    
    //BR
    _quadFSVAR[20] = qright;
    _quadFSVAR[21] = qbottom;
    _quadFSVAR[22] = qendU;
    _quadFSVAR[23] = qstartV;
}


void VROSceneRendererCardboardMetal::setSuspended(bool suspended) {
    _suspended = suspended;
}

#endif
