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
#include "VRODistortionMesh.h"
#include "VROEye.h"
#include "VRODevice.h"

#pragma mark - Initialization

VRODistortionRenderer::VRODistortionRenderer(VRODevice &device) :
    _resolutionScale(1.0f),
    _chromaticAberrationCorrectionEnabled(false),
    _vignetteEnabled(true),
    _leftEyeDistortionMesh(nullptr),
    _rightEyeDistortionMesh(nullptr),
    _device(device),
    _fovsChanged(false),
    _viewportsChanged(false),
    _drawingFrame(false),
    _xPxPerTanAngle(0),
    _yPxPerTanAngle(0),
    _metersPerTanAngle(0) {
}

VRODistortionRenderer::~VRODistortionRenderer() {
    if (_leftEyeDistortionMesh != nullptr) {
        delete (_leftEyeDistortionMesh);
    }
    if (_rightEyeDistortionMesh != nullptr) {
        delete (_rightEyeDistortionMesh);
    }
}

#pragma mark - Viewport Computation

void VRODistortionRenderer::fovDidChange(const VROFieldOfView &leftEyeFov,
                                         const VROFieldOfView &rightEyeFov,
                                         float virtualEyeToScreenDistance) {
    if (_drawingFrame) {
        NSLog(@"Cannot change FOV while rendering a frame.");
        return;
    }
    
    _leftEyeViewport = { leftEyeFov, 0.0f };
    _rightEyeViewport = { rightEyeFov, _leftEyeViewport.width };
    _metersPerTanAngle = virtualEyeToScreenDistance;
    
    const VROScreen &screen = _device.getScreen();
        
    _xPxPerTanAngle = screen.getWidth()  / (screen.getWidthInMeters()  / _metersPerTanAngle);
    _yPxPerTanAngle = screen.getHeight() / (screen.getHeightInMeters() / _metersPerTanAngle);
    _fovsChanged = true;
    _viewportsChanged = true;
}

VROEyeViewport::VROEyeViewport(const VROFieldOfView &eyeFOV, float xOffset) {
    float left   = tanf(GLKMathDegreesToRadians(eyeFOV.getLeft()));
    float right  = tanf(GLKMathDegreesToRadians(eyeFOV.getRight()));
    float bottom = tanf(GLKMathDegreesToRadians(eyeFOV.getBottom()));
    float top    = tanf(GLKMathDegreesToRadians(eyeFOV.getTop()));
    
    x = xOffset;
    y = 0.0f;
    width = (left + right);
    height = (bottom + top);
    eyeX = (left + xOffset);
    eyeY = bottom;
}

void VRODistortionRenderer::updateViewports(VROEye *leftEye, VROEye *rightEye) {
    leftEye->setViewport(round(_leftEyeViewport.x * _xPxPerTanAngle * _resolutionScale),
                         round(_leftEyeViewport.y * _yPxPerTanAngle * _resolutionScale),
                         round(_leftEyeViewport.width * _xPxPerTanAngle * _resolutionScale),
                         round(_leftEyeViewport.height * _yPxPerTanAngle * _resolutionScale));
    rightEye->setViewport(round(_rightEyeViewport.x * _xPxPerTanAngle * _resolutionScale),
                          round(_rightEyeViewport.y * _yPxPerTanAngle * _resolutionScale),
                          round(_rightEyeViewport.width * _xPxPerTanAngle * _resolutionScale),
                          round(_rightEyeViewport.height * _yPxPerTanAngle * _resolutionScale));
    _viewportsChanged = false;
}

void VRODistortionRenderer::updateTextureAndDistortionMesh(const VRORenderContextMetal &metal) {
    id <MTLDevice> gpu = metal.getDevice();
    
    const VROScreen &screen = _device.getScreen();
    
    /*
     Compute the size of the required eye render texture (the texture to which we render
     BOTH eyes), and create it.
     */
    float textureWidthTanAngle  = _leftEyeViewport.width + _rightEyeViewport.width;
    float textureHeightTanAngle = MAX(_leftEyeViewport.height, _rightEyeViewport.height);
    int maxTextureSize = 2048; // TODO query GPU to find this
    
    int textureWidthPx  = MIN(round(_leftEyeViewport.width * _xPxPerTanAngle) +
                                round(_rightEyeViewport.width * _xPxPerTanAngle),
                                maxTextureSize);
    int textureHeightPx = MIN(round(MAX(_leftEyeViewport.height, _rightEyeViewport.height) * _yPxPerTanAngle),
                                maxTextureSize);
    
    float xEyeOffsetTanAngleScreen = (screen.getWidthInMeters() / 2.0f - _device.getInterLensDistance() / 2.0f) / _metersPerTanAngle;
    float yEyeOffsetTanAngleScreen = (_device.getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters()) / _metersPerTanAngle;
    
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:textureWidthPx
                                                                                         height:textureHeightPx
                                                                                      mipmapped:NO];
    _texture = [gpu newTextureWithDescriptor:descriptor];
    
    /*
     Create the corresponding distortion meshes.
     */
    _leftEyeDistortionMesh = createDistortionMesh(_leftEyeViewport,
                                                  textureWidthTanAngle, textureHeightTanAngle,
                                                  xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen, gpu);
    xEyeOffsetTanAngleScreen = screen.getWidthInMeters() / _metersPerTanAngle - xEyeOffsetTanAngleScreen;
    
    _rightEyeDistortionMesh = createDistortionMesh(_rightEyeViewport,
                                                   textureWidthTanAngle, textureHeightTanAngle,
                                                   xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen, gpu);
    
    
    updateDistortionPassPipeline(metal);
    _fovsChanged = false;
}

#pragma mark - Rendering Eye Texture

id <MTLRenderCommandEncoder> VRODistortionRenderer::bindEyeRenderTarget(const VRORenderContextMetal &metal) {
    _drawingFrame = true;
    
    if (_fovsChanged) {
        updateTextureAndDistortionMesh(metal);
    }
    
    return createEyeRenderEncoder(metal);
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

void VRODistortionRenderer::updateDistortionPassPipeline(const VRORenderContextMetal &metal) {
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

#pragma mark - Rendering Eye Texture to Distortion Mesh

void VRODistortionRenderer::renderEyesToScreen(const VRORenderContextMetal &metal) {
    if (_fovsChanged) {
        updateTextureAndDistortionMesh(metal);
    }
    
    const VROScreen &screen = _device.getScreen();
    
    id <MTLRenderCommandEncoder> screenEncoder = metal.getRenderEncoder();
    [screenEncoder setDepthStencilState:_depthState];
    [screenEncoder setViewport: { 0, 0,
        (double) screen.getWidth(),
        (double) screen.getHeight(),
        0, 1}];
    
    if (_chromaticAberrationCorrectionEnabled) {
        // TODO set the aberration pipeline state
    }
    else {
        [screenEncoder setRenderPipelineState:_pipelineState];
    }
    
    [screenEncoder setScissorRect:{ 0, 0,
        (NSUInteger) (screen.getWidth() / 2),
        (NSUInteger) (screen.getHeight()) }];
    renderDistortionMesh(*_leftEyeDistortionMesh, _texture, screenEncoder);
    
    [screenEncoder setScissorRect:{ (NSUInteger) screen.getWidth() / 2, 0,
        (NSUInteger) screen.getWidth() / 2,
        (NSUInteger) screen.getHeight() }];
    renderDistortionMesh(*_rightEyeDistortionMesh, _texture, screenEncoder);
    
    _drawingFrame = false;
}

VRODistortionMesh *VRODistortionRenderer::createDistortionMesh(const VROEyeViewport &eyeViewport,
                                                               float textureWidthTanAngle,
                                                               float textureHeightTanAngle,
                                                               float xEyeOffsetTanAngleScreen,
                                                               float yEyeOffsetTanAngleScreen,
                                                               id <MTLDevice> gpu) {
    return new VRODistortionMesh(_device.getDistortion(),
                                 _device.getDistortion(),
                                 _device.getDistortion(),
                                 _device.getScreen().getWidthInMeters() / _metersPerTanAngle,
                                 _device.getScreen().getHeightInMeters() / _metersPerTanAngle,
                                 xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen,
                                 textureWidthTanAngle, textureHeightTanAngle,
                                 eyeViewport.eyeX, eyeViewport.eyeY,
                                 eyeViewport.x, eyeViewport.y,
                                 eyeViewport.width, eyeViewport.height,
                                 _vignetteEnabled,
                                 gpu);
}

void VRODistortionRenderer::renderDistortionMesh(const VRODistortionMesh &mesh, id <MTLTexture> texture,
                                                 id <MTLRenderCommandEncoder> renderEncoder) {
    
    VRODistortionUniforms *uniforms = (VRODistortionUniforms *)[_uniformsBuffer contents];
    uniforms->texcoord_scale = _resolutionScale;
    
    [renderEncoder pushDebugGroup:@"VRODistortion"];
    
    [renderEncoder setVertexBuffer:_uniformsBuffer offset:0 atIndex:1];
    [renderEncoder setFragmentTexture:_texture atIndex:0];
    mesh.render(renderEncoder);
    
    [renderEncoder popDebugGroup];
}

float VRODistortionRenderer::computeDistortionScale(const VRODistortion &distortion,
                                                    float screenWidthM,
                                                    float interpupillaryDistanceM) {
    return distortion.getDistortionFactor((screenWidthM / 2.0f - interpupillaryDistanceM / 2.0f) / (screenWidthM / 4.0f));
}