//
//  VRODistortionRenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRODistortionRenderer.h"
#if VRO_METAL

#include "VROMath.h"
#include "VROViewport.h"
#include "VROScreen.h"
#include "VROSharedStructures.h"
#include "VRODistortionMesh.h"
#include "VROEye.h"
#include "VRODevice.h"
#include "VROConcurrentBuffer.h"
#include "VROViewMetal.h"

static const float kSampleCount = 4;

#pragma mark - Initialization

VRODistortionRenderer::VRODistortionRenderer(std::shared_ptr<VRODevice> device) :
    _resolutionScale(1.0f),
    _chromaticAberrationCorrectionEnabled(false),
    _vignetteEnabled(true),
    _leftEyeDistortionMesh(nullptr),
    _rightEyeDistortionMesh(nullptr),
    _device(device),
    _fovsChanged(false),
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

void VRODistortionRenderer::fovDidChange(VROEye *leftEye, VROEye *rightEye,
                                         float virtualEyeToScreenDistance) {
    if (_drawingFrame) {
        NSLog(@"Cannot change FOV while rendering a frame.");
        return;
    }
    
    const VROFieldOfView &leftEyeFov = leftEye->getFOV();
    const VROFieldOfView &rightEyeFov = rightEye->getFOV();
    
    _leftEyeViewport = { leftEyeFov, 0.0f };
    _rightEyeViewport = { rightEyeFov, _leftEyeViewport.width };
    
    /*
             Eye
              .
              ..  <---- angle
              . .
            y .  . r
              .   .
              .    .
        -------------
                 x
     
     
       tan(angle) = x/y
       unitPerTanAngle = x/tan(angle) = y, where 'unit' is meters or pixels
     
       In meters, this means: 
     
        y = virtualEyeToScreenDistance

       (1) tan(angle) = screen.getWidthInMeters() / virtualEyeToScreenDistance
       (2) metersPerTanAngle = screen.getWidthInMeters()/tan(angle) = virtualEyeToScreenDistance

       In pixels, we have the analagous formula:
     
       (3) pixelsPerTanAngle = screen.getWidth() / tan(angle)
     
       Substituting in (3), we get:
       pixelsPerTanAngle = screen.getWidth() / (screen.getWidthInMeters() / virtualEyeToScreenDistance)
     */
    
    _metersPerTanAngle = virtualEyeToScreenDistance;
    
    const VROScreen &screen = _device->getScreen();
    _xPxPerTanAngle = screen.getWidth()  / (screen.getWidthInMeters()  / virtualEyeToScreenDistance);
    _yPxPerTanAngle = screen.getHeight() / (screen.getHeightInMeters() / virtualEyeToScreenDistance);
    _fovsChanged = true;
    
    updateViewports(leftEye, rightEye);
}

VROEyeViewport::VROEyeViewport(const VROFieldOfView &eyeFOV, float xOffset) {
    float left   = tanf(toRadians(eyeFOV.getLeft()));
    float right  = tanf(toRadians(eyeFOV.getRight()));
    float bottom = tanf(toRadians(eyeFOV.getBottom()));
    float top    = tanf(toRadians(eyeFOV.getTop()));
    
    x = xOffset;
    y = 0.0f;
    width = (left + right);
    height = (bottom + top);
    eyeX = (left + xOffset);
    eyeY = bottom;
}

void VRODistortionRenderer::updateViewports(VROEye *leftEye, VROEye *rightEye) {
    float maxWidth = 1024;//_device->getScreen().getWidth() / 2.0;
    
    leftEye->setViewport(round(_leftEyeViewport.x * _xPxPerTanAngle * _resolutionScale),
                         round(_leftEyeViewport.y * _yPxPerTanAngle * _resolutionScale),
                         MIN(maxWidth, round(_leftEyeViewport.width * _xPxPerTanAngle * _resolutionScale)),
                         round(_leftEyeViewport.height * _yPxPerTanAngle * _resolutionScale));
    rightEye->setViewport(MIN(maxWidth, round(_rightEyeViewport.x * _xPxPerTanAngle * _resolutionScale)),
                          round(_rightEyeViewport.y * _yPxPerTanAngle * _resolutionScale),
                          MIN(maxWidth, round(_rightEyeViewport.width * _xPxPerTanAngle * _resolutionScale)),
                          round(_rightEyeViewport.height * _yPxPerTanAngle * _resolutionScale));
}

void VRODistortionRenderer::updateDistortion(id <MTLDevice> gpu, id <MTLLibrary> library, VROViewMetal *view) {
    if (!_fovsChanged) {
        return;
    }
    const VROScreen &screen = _device->getScreen();
    
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
    
    float xEyeOffsetTanAngleScreen = (screen.getWidthInMeters() / 2.0f - _device->getInterLensDistance() / 2.0f) / _metersPerTanAngle;
    float yEyeOffsetTanAngleScreen = (_device->getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters()) / _metersPerTanAngle;
    
    MTLTextureDescriptor *msaaDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:textureWidthPx
                                                                                       height:textureHeightPx
                                                                                    mipmapped:NO];
    msaaDesc.textureType = MTLTextureType2DMultisample;
    msaaDesc.sampleCount = kSampleCount;
    
    _msaaTexture = [gpu newTextureWithDescriptor:msaaDesc];
    
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
    
    
    updateDistortionPassPipeline(gpu, library, view);
    _fovsChanged = false;
}

#pragma mark - Rendering Eye Texture

std::shared_ptr<VRORenderTarget> VRODistortionRenderer::bindEyeRenderTarget(id <MTLCommandBuffer> commandBuffer) {
    _drawingFrame = true;
    
    MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture = _msaaTexture;
    renderPassDesc.colorAttachments[0].resolveTexture = _texture;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
    eyeRenderEncoder.label = @"EyeRenderEncoder";
    
    [eyeRenderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [eyeRenderEncoder setCullMode:MTLCullModeBack];
    
    return std::make_shared<VRORenderTarget>(eyeRenderEncoder, _texture.pixelFormat, MTLPixelFormatInvalid, _msaaTexture.sampleCount);
}

void VRODistortionRenderer::updateDistortionPassPipeline(id <MTLDevice> gpu, id <MTLLibrary> library, MTKView *view) {
    /*
     Set up the pipeline for rendering to the eye texture.
     */
    _uniformsBuffer = new VROConcurrentBuffer(sizeof(VRODistortionUniforms), @"VRODistortionUniformBuffer", gpu);
    
    id <MTLFunction> fragmentProgram = [library newFunctionWithName:@"distortion_fragment"];
    id <MTLFunction> vertexProgram   = [library newFunctionWithName:@"distortion_vertex"];
    
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
    pipelineStateDescriptor.sampleCount = view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
    
    NSError *error = NULL;
    _pipelineState = [gpu newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;
    
    _depthState = [gpu newDepthStencilStateWithDescriptor:depthStateDesc];
}

#pragma mark - Rendering Eye Texture to Distortion Mesh

void VRODistortionRenderer::renderEyesToScreen(id <MTLRenderCommandEncoder> screenEncoder, int frame) {
    const VROScreen &screen = _device->getScreen();
    
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
    renderDistortionMesh(*_leftEyeDistortionMesh, _texture, screenEncoder, VROEyeType::Left, frame);
    
    [screenEncoder setScissorRect:{ (NSUInteger) screen.getWidth() / 2, 0,
        (NSUInteger) screen.getWidth() / 2,
        (NSUInteger) screen.getHeight() }];
    renderDistortionMesh(*_rightEyeDistortionMesh, _texture, screenEncoder, VROEyeType::Right, frame);
    
    _drawingFrame = false;
}

VRODistortionMesh *VRODistortionRenderer::createDistortionMesh(const VROEyeViewport &eyeViewport,
                                                               float textureWidthTanAngle,
                                                               float textureHeightTanAngle,
                                                               float xEyeOffsetTanAngleScreen,
                                                               float yEyeOffsetTanAngleScreen,
                                                               id <MTLDevice> gpu) {
    return new VRODistortionMesh(_device->getDistortion(),
                                 _device->getDistortion(),
                                 _device->getDistortion(),
                                 _device->getScreen().getWidthInMeters() / _metersPerTanAngle,
                                 _device->getScreen().getHeightInMeters() / _metersPerTanAngle,
                                 xEyeOffsetTanAngleScreen, yEyeOffsetTanAngleScreen,
                                 textureWidthTanAngle, textureHeightTanAngle,
                                 eyeViewport.eyeX, eyeViewport.eyeY,
                                 eyeViewport.x, eyeViewport.y,
                                 eyeViewport.width, eyeViewport.height,
                                 _vignetteEnabled,
                                 gpu);
}

void VRODistortionRenderer::renderDistortionMesh(const VRODistortionMesh &mesh,
                                                 id <MTLTexture> texture,
                                                 id <MTLRenderCommandEncoder> renderEncoder,
                                                 VROEyeType eye,
                                                 int frame) {
    
    VRODistortionUniforms *uniforms = (VRODistortionUniforms *)_uniformsBuffer->getWritableContents(eye, frame);
    uniforms->texcoord_scale = _resolutionScale;
    
    [renderEncoder pushDebugGroup:@"VRODistortion"];
    
    [renderEncoder setVertexBuffer:_uniformsBuffer->getMTLBuffer(eye)
                            offset:_uniformsBuffer->getWriteOffset(frame)
                           atIndex:1];
    [renderEncoder setFragmentTexture:_texture atIndex:0];
    mesh.render(renderEncoder);
    
    [renderEncoder popDebugGroup];
}

float VRODistortionRenderer::computeDistortionScale(const VRODistortion &distortion,
                                                    float screenWidthM,
                                                    float interpupillaryDistanceM) {
    return distortion.getDistortionFactor((screenWidthM / 2.0f - interpupillaryDistanceM / 2.0f) / (screenWidthM / 4.0f));
}

#endif