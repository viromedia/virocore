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
#include "SharedStructures.h"
#include "VRORenderContextMetal.h"

VRODistortionRenderer::VRODistortionRenderer() :
    _resolutionScale(1.0f),
    _chromaticAberrationCorrectionEnabled(false),
    _vignetteEnabled(true),
    _leftEyeDistortionMesh(nullptr),
    _rightEyeDistortionMesh(nullptr),
    _headMountedDisplay(nullptr),
    _leftEyeViewport(nullptr),
    _rightEyeViewport(nullptr),
    _fovsChanged(false),
    _viewportsChanged(false),
    _drawingFrame(false),
    _xPxPerTanAngle(0),
    _yPxPerTanAngle(0),
    _metersPerTanAngle(0) {
}

VRODistortionRenderer::~VRODistortionRenderer() {
    if (_leftEyeDistortionMesh != nullptr) {
        delete _leftEyeDistortionMesh;
    }
    if (_rightEyeDistortionMesh != nullptr) {
        delete _rightEyeDistortionMesh;
    }
    
    if (_leftEyeViewport != nullptr) {
        delete _leftEyeViewport;
    }
    if (_rightEyeViewport != nullptr) {
        delete _rightEyeViewport;
    }
}

id <MTLRenderCommandEncoder> VRODistortionRenderer::bindEyeRenderTarget(const VRORenderContextMetal &metal) {
    _drawingFrame = true;
    
    if (_fovsChanged) {
        updateTextureAndDistortionMesh(metal);
    }
    
    return createEyeRenderEncoder(metal);
}

void VRODistortionRenderer::renderEyesToScreen(const VRORenderContextMetal &metal) {
    if (_fovsChanged) {
        updateTextureAndDistortionMesh(metal);
    }
    
    std::shared_ptr<VROScreen> screen = _headMountedDisplay->getScreen();
    
    id <MTLRenderCommandEncoder> screenEncoder = metal.getRenderEncoder();
    [screenEncoder setDepthStencilState:_depthState];
    [screenEncoder setViewport: { 0, 0,
                                  (double) screen->getWidth(),
                                  (double) screen->getHeight(),
                                  0, 1}];

    if (_chromaticAberrationCorrectionEnabled) {
        // TODO set the aberration pipeline state
    }
    else {
        [screenEncoder setRenderPipelineState:_pipelineState];
    }
    
    [screenEncoder setScissorRect:{ 0, 0,
                                    (NSUInteger) (screen->getWidth() / 2),
                                    (NSUInteger) (screen->getHeight()) }];
    renderDistortionMesh(_leftEyeDistortionMesh, _texture, screenEncoder);
    
    [screenEncoder setScissorRect:{ (NSUInteger) screen->getWidth() / 2, 0,
                                    (NSUInteger) screen->getWidth() / 2,
                                    (NSUInteger) screen->getHeight() }];
    renderDistortionMesh(_rightEyeDistortionMesh, _texture, screenEncoder);
    
    _drawingFrame = false;
}

void VRODistortionRenderer::fovDidChange(VROHeadMountedDisplay *headMountedDisplay,
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
        
    _xPxPerTanAngle = screen->getWidth()  / (screen->getWidthInMeters()  / _metersPerTanAngle);
    _yPxPerTanAngle = screen->getHeight() / (screen->getHeightInMeters() / _metersPerTanAngle);
    _fovsChanged = true;
    _viewportsChanged = true;
}

void VRODistortionRenderer::updateViewports(VROViewport *leftViewport, VROViewport *rightViewport) {
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

void VRODistortionRenderer::updateTextureAndDistortionMesh(const VRORenderContextMetal &metal) {
    id <MTLDevice> gpu = metal.getDevice();
    
    std::shared_ptr<VROScreen> screen = _headMountedDisplay->getScreen();
    std::shared_ptr<VRODevice> device = _headMountedDisplay->getDevice();
    
    /*
     Compute the size of the required eye render texture (the texture to which we render
     BOTH eyes), and create it.
     */
    float textureWidthTanAngle  = _leftEyeViewport->width + _rightEyeViewport->width;
    float textureHeightTanAngle = MAX(_leftEyeViewport->height, _rightEyeViewport->height);
    GLint maxTextureSize = 2048; // TODO query GPU to find this
    
    GLint textureWidthPx  = MIN(round(_leftEyeViewport->width * _xPxPerTanAngle) +
                                round(_rightEyeViewport->width * _xPxPerTanAngle),
                                maxTextureSize);
    GLint textureHeightPx = MIN(round(MAX(_leftEyeViewport->height, _rightEyeViewport->height) * _yPxPerTanAngle),
                                maxTextureSize);
    
    float xEyeOffsetTanAngleScreen = (screen->getWidthInMeters() / 2.0f - device->getInterLensDistance() / 2.0f) / _metersPerTanAngle;
    float yEyeOffsetTanAngleScreen = (device->getVerticalDistanceToLensCenter() - screen->getBorderSizeInMeters()) / _metersPerTanAngle;
    
    setupRenderTexture(textureWidthPx, textureHeightPx, gpu);

    /*
     Create the corresponding distortion meshes.
     */
    _leftEyeDistortionMesh = createDistortionMesh(_leftEyeViewport,
                                                  textureWidthTanAngle, textureHeightTanAngle,
                                                  xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen, gpu);
    xEyeOffsetTanAngleScreen = screen->getWidthInMeters() / _metersPerTanAngle - xEyeOffsetTanAngleScreen;
    
    _rightEyeDistortionMesh = createDistortionMesh(_rightEyeViewport,
                                                   textureWidthTanAngle, textureHeightTanAngle,
                                                   xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen, gpu);
    
    
    updateDistortionMeshPipeline(metal);
    _fovsChanged = false;
}

id <MTLRenderCommandEncoder> VRODistortionRenderer::createEyeRenderEncoder(const VRORenderContextMetal &metal) {
    MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = _texture;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = [metal.getCommandBuffer() renderCommandEncoderWithDescriptor:renderPassDesc];
    eyeRenderEncoder.label = @"EyeRenderEncoder";
    
    return eyeRenderEncoder;
}

void VRODistortionRenderer::updateDistortionMeshPipeline(const VRORenderContextMetal &metal) {
    /*
     Set up the pipeline for rendering to the eye texture.
     */
    _uniformsBuffer = [metal.getDevice() newBufferWithLength:sizeof(VRODistortionUniforms) options:0];
    _uniformsBuffer.label = @"VRODistortionUniformBuffer";
    
    id <MTLFunction> fragmentProgram = [metal.getLibrary() newFunctionWithName:@"distortion_fragment"];
    id <MTLFunction> vertexProgram   = [metal.getLibrary() newFunctionWithName:@"distortion_vertex"];
    
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat;
    vertexDescriptor.attributes[1].offset = sizeof(float) * 2;
    vertexDescriptor.attributes[1].bufferIndex = 0;
    
    vertexDescriptor.attributes[2].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[2].offset = sizeof(float) * (2 + 1);
    vertexDescriptor.attributes[2].bufferIndex = 0;
    
    vertexDescriptor.attributes[3].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[3].offset = sizeof(float) * (2 + 1 + 2);
    vertexDescriptor.attributes[3].bufferIndex = 0;
    
    vertexDescriptor.attributes[4].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[4].offset = sizeof(float) * (2 + 1 + 2 + 2);
    vertexDescriptor.attributes[4].bufferIndex = 0;
    
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stride = sizeof(float) * 9;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"VRODistortionPipeline";
    pipelineStateDescriptor.sampleCount = metal.getSampleCount();
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = metal.getColorPixelFormat();
    pipelineStateDescriptor.depthAttachmentPixelFormat = metal.getDepthStencilPixelFormat();
    pipelineStateDescriptor.stencilAttachmentPixelFormat = metal.getDepthStencilPixelFormat();
    
    NSError *error = NULL;
    _pipelineState = [metal.getDevice() newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;
    
    _depthState = [metal.getDevice() newDepthStencilStateWithDescriptor:depthStateDesc];
}

VRODistortionRenderer::EyeViewport *VRODistortionRenderer::initViewportForEye(VROFieldOfView *eyeFieldOfView, float xOffset) {
    float left   = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getLeft()));
    float right  = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getRight()));
    float bottom = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getBottom()));
    float top    = tanf(GLKMathDegreesToRadians(eyeFieldOfView->getTop()));
    
    EyeViewport *eyeViewport = new EyeViewport();
    eyeViewport->x = xOffset;
    eyeViewport->y = 0.0f;
    eyeViewport->width = (left + right);
    eyeViewport->height = (bottom + top);
    eyeViewport->eyeX = (left + xOffset);
    eyeViewport->eyeY = bottom;
    
    return eyeViewport;
}

VRODistortionRenderer::DistortionMesh *VRODistortionRenderer::createDistortionMesh(EyeViewport *eyeViewport,
                                                                             float textureWidthTanAngle,
                                                                             float textureHeightTanAngle,
                                                                             float xEyeOffsetTanAngleScreen,
                                                                             float yEyeOffsetTanAngleScreen,
                                                                             id <MTLDevice> gpu) {
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
                              _vignetteEnabled,
                              gpu);
}

void VRODistortionRenderer::renderDistortionMesh(DistortionMesh *mesh, id <MTLTexture> texture,
                                                 id <MTLRenderCommandEncoder> renderEncoder) {
    
    VRODistortionUniforms *uniforms = (VRODistortionUniforms *)[_uniformsBuffer contents];
    uniforms->texcoord_scale = _resolutionScale;
    
    [renderEncoder pushDebugGroup:@"VRODistortion"];
    
    [renderEncoder setVertexBuffer:mesh->_vertexBuffer offset:0 atIndex:0];
    [renderEncoder setVertexBuffer:_uniformsBuffer offset:0 atIndex:1];
    [renderEncoder setFragmentTexture:_texture atIndex:0];
    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip
                              indexCount:[mesh->_indexBuffer length] / sizeof(short)
                               indexType:MTLIndexTypeUInt16
                             indexBuffer:mesh->_indexBuffer
                       indexBufferOffset:0];
    [renderEncoder popDebugGroup];
}

float VRODistortionRenderer::computeDistortionScale(VRODistortion *distortion, float screenWidthM, float interpupillaryDistanceM) {
    return distortion->getDistortionFactor((screenWidthM / 2.0f - interpupillaryDistanceM / 2.0f) / (screenWidthM / 4.0f));
}

void VRODistortionRenderer::setupRenderTexture(int width, int height, id <MTLDevice> gpu) {
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:width
                                                                                         height:height
                                                                                      mipmapped:NO];
    _texture = [gpu newTextureWithDescriptor:descriptor];
}

#pragma mark - Distortion Mesh

VRODistortionRenderer::DistortionMesh::DistortionMesh(VRODistortion *distortionRed,
                                                      VRODistortion *distortionGreen,
                                                      VRODistortion *distortionBlue,
                                                      float screenWidth, float screenHeight,
                                                      float xEyeOffsetScreen, float yEyeOffsetScreen,
                                                      float textureWidth, float textureHeight,
                                                      float xEyeOffsetTexture, float yEyeOffsetTexture,
                                                      float viewportXTexture, float viewportYTexture,
                                                      float viewportWidthTexture, float viewportHeightTexture,
                                                      bool vignetteEnabled,
                                                      id <MTLDevice> gpu) {
    float vertexData[14400];
    int vertexOffset = 0;
    
    const int rows = 40;
    const int cols = 40;
    
    const float vignetteSizeTanAngle = 0.05f;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
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
            if (vignetteEnabled) {
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
    
    int numIndices = 3158;
    uint16_t indexData[numIndices];
    
    int indexOffset = 0;
    vertexOffset = 0;
    for (int row = 0; row < rows-1; row++) {
        if (row > 0) {
            indexData[indexOffset] = indexData[(indexOffset - 1)];
            indexOffset++;
        }
        for (int col = 0; col < cols; col++) {
            if (col > 0) {
                if (row % 2 == 0) {
                    vertexOffset++;
                }
                else {
                    vertexOffset--;
                }
            }
            indexData[(indexOffset++)] = vertexOffset;
            indexData[(indexOffset++)] = (vertexOffset + 40);
        }
        vertexOffset += 40;
    }
    
    _vertexBuffer = [gpu newBufferWithLength:sizeof(vertexData) options:0];
    _vertexBuffer.label = @"VRODistortionMeshVertexBuffer";
    
    // TODO Switch to using newBufferWithBytesNoCopy?
    memcpy([_vertexBuffer contents], vertexData, sizeof(vertexData));
    
    _indexBuffer = [gpu newBufferWithLength:sizeof(indexData) options:0];
    _indexBuffer.label = @"VRODistortionMeshIndexBuffer";
    
    memcpy([_indexBuffer contents], indexData, sizeof(indexData));
}