//
//  VROViewCardboard.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROViewCardboard.h"
#import <memory>
#import "VRORenderer.h"
#import "VROViewport.h"
#import "VROEye.h"
#import "VRODriverCardboard.h"
#import "VRODriverContextMetal.h"
#import "VRORenderTarget.h"
#import "VROCardboardRenderLoop.h"
#import "VROShaderProgram.h"

#import <GLKit/GLKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

static const float kSampleCount = 4;
static const MTLPixelFormat kResolvePixelFormat = MTLPixelFormatRGBA8Unorm;

@interface VROViewCardboard () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriverContextMetal> _context;
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
}

@property (readwrite, nonatomic) int frameNumber;
@property (readwrite, nonatomic) std::shared_ptr<VRODriver> driver;
@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;

@end

@implementation VROViewCardboard

@dynamic renderDelegate;

#pragma mark - Initialization

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (void)initRenderer {
    self.frameNumber = 0;
    self.delegate = self;
    
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    self.renderer = std::make_shared<VRORenderer>();
    self.driver = std::make_shared<VRODriverCardboard>(self.renderer);

    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    _context = std::make_shared<VRODriverContextMetal>(device);

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                                                    action:@selector(handleTap:)];
    [self addGestureRecognizer:tapRecognizer];
}

- (void)dealloc {
    free (_textureBuffer);
    delete (_blitter);
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Settings

- (void)orientationDidChange:(NSNotification *)notification {
    self.driver->onOrientationChange([UIApplication sharedApplication].statusBarOrientation);
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    self.renderer->setDelegate(renderDelegate);
}

#pragma mark - Camera

- (void)setPosition:(VROVector3f)position {
    self.renderer->setPosition(position);
}

- (void)setBaseRotation:(VROQuaternion)rotation {
    self.renderer->setBaseRotation(rotation);
}

- (float)worldPerScreenAtDepth:(float)distance {
    return self.renderer->getWorldPerScreen(distance,
                                            self.driver->getFOV(VROEyeType::Left),
                                            self.driver->getViewport(VROEyeType::Left));
}

- (void)layoutSubviews {
    [super layoutSubviews];
    _renderer->updateRenderViewSize(self.bounds.size);
}

#pragma mark - Reticle

- (void)handleTap:(UIGestureRecognizer *)gestureRecognizer {
    _renderer->handleTap();
}

- (VROScreenUIView *)HUD {
    return _renderer->getHUD();
}

#pragma mark - Scene Loading

- (VROSceneController *)sceneController {
    return _renderer->getSceneController();
}

- (void)setSceneController:(VROSceneController *)sceneController {
    _renderer->setSceneController(sceneController);
}

- (void)setSceneController:(VROSceneController *)sceneController animated:(BOOL)animated {
    _renderer->setSceneController(sceneController, animated);
}

- (void)setSceneController:(VROSceneController *)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    
    _renderer->setSceneController(sceneController, seconds, timingFunctionType);
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer;
}

#pragma mark - Cardboard View Delegate

- (void)cardboardView:(GCSCardboardView *)cardboardView didFireEvent:(GCSUserEvent)event {
    
}

- (void)cardboardView:(GCSCardboardView *)cardboardView
     willStartDrawing:(GCSHeadTransform *)headTransform {
    
    id <MTLDevice> gpu = _context->getDevice();
    
    /*
     Compute the size of the required eye render texture (the texture to which we render
     BOTH eyes), and create it.
     */
    int maxTextureSize = 2048; // TODO query GPU to find this
    
    CGRect leftViewport  = [headTransform viewportForEye:kGCSLeftEye];
    CGRect rightViewport = [headTransform viewportForEye:kGCSRightEye];
    
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
    
    [self buildFullScreenQuadVAR];
    glGenTextures(1, &_textureGL);
    
    uint32_t bytesPerPixel = 4;
    uint64_t bytesPerRow = bytesPerPixel * [_texture width];
    _textureBuffer = (char *) malloc(bytesPerRow * [_texture height]);
    
    _blitter = new VROShaderProgram("blit", 0);
    
    const char *samplers[1];
    samplers[0] = "map0";
    _blitter->setSamplers(samplers, 1);
    _blitter->hydrate();
}

- (void)cardboardView:(GCSCardboardView *)cardboardView
     prepareDrawFrame:(GCSHeadTransform *)headTransform {
    
    VRODriverContextMetal *driverContext = (VRODriverContextMetal *)_context.get();
    
    _commandBuffer = [driverContext->getCommandQueue() commandBuffer];
    _commandBuffer.label = @"CommandBuffer";
    
    _eyeTarget = [self createEyeRenderTarget];
    _context->setRenderTarget(_eyeTarget);
}

/**
 * Called on each frame to perform the required GL rendering. Delegate should set the GL viewport
 * and scissor it to the viewport returned from |GCSHeadTransforms|'s |viewportForEye| method.
 * This method is called on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
              drawEye:(GCSEye)eye
    withHeadTransform:(GCSHeadTransform *)headTransform {
    
    CGRect rect = [headTransform viewportForEye:eye];
    VROViewport viewport(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    
    VROMatrix4f headRotation = matrix_float4x4_from_GL([headTransform headPoseInStartSpace]);
    _renderer->prepareFrame(_frameNumber, headRotation, *_context.get());
    
    VROMatrix4f eyeMatrix = matrix_float4x4_from_GL([headTransform eyeFromHeadMatrix:eye]);
    VROMatrix4f projectionMatrix = matrix_float4x4_from_GL([headTransform projectionMatrixForEye:eye near:0.01 far:100]); //TODO Near far
    
    id <MTLRenderCommandEncoder> renderEncoder = _eyeTarget->getRenderEncoder();
    [renderEncoder setViewport:viewport.toMetalViewport()];
    [renderEncoder setScissorRect:viewport.toMetalScissor()];
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    
    VROEyeType eyeType = (eye == kGCSLeftEye ? VROEyeType::Left : VROEyeType::Right);
    _renderer->renderEye(eyeType, eyeMatrix, projectionMatrix, *_context.get());
    
    // End frame
    if (eye == kGCSRightEye || eye == kGCSCenterEye) {
        _renderer->endFrame(*_context.get());
        [renderEncoder endEncoding];
        
        [self writeGLTexture];
        [self drawTexture];
        
        [_commandBuffer commit];
        //[_context->getCommandQueue() insertDebugCaptureBoundary];
        
        ++_frameNumber;
    }
}

- (std::shared_ptr<VRORenderTarget>) createEyeRenderTarget {
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

- (void)writeGLTexture {
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

- (UIImage *)writeTextureToImage {
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

- (void)drawTexture {
    glDisable(GL_DEPTH_TEST);
    
    glBindTexture(GL_TEXTURE_2D, _textureGL);
    [self drawScreenSpaceVAR];
    
    glEnable(GL_DEPTH_TEST);
}

- (void)drawScreenSpaceVAR {
    glDisable(GL_CULL_FACE);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    _blitter->bind();
    
    glEnableVertexAttribArray((int) VROShaderAttribute::Verts);
    glEnableVertexAttribArray((int) VROShaderAttribute::Tex);
    
    glVertexAttribPointer((int) VROShaderAttribute::Verts, 2, GL_FLOAT, 0, 16, _quadFSVAR);
    glVertexAttribPointer((int) VROShaderAttribute::Tex, 2, GL_FLOAT, 0, 16, ((char *) _quadFSVAR + 2 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)buildFullScreenQuadVAR {
    float qstartV = 0.0f;
    float qendV = 1.0f;
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

@end
