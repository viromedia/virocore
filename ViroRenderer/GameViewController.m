//
//  GameViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "GameViewController.h"
#import "SharedStructures.h"

#import "VRORenderContextMetal.h"
#import "VROLayer.h"
#import "VROMath.h"

// The max number of command buffers in flight
static const NSUInteger kMaxInflightBuffers = 3;

@implementation GameViewController
{
    // view
    MTKView *_view;
    
    VRORenderContextMetal *_renderContext;
    VROLayer *_layerA;
    VROLayer *_layerB;
    
    dispatch_semaphore_t _inflight_semaphore;
    uint8_t _constantDataBufferIndex;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _constantDataBufferIndex = 0;
    _inflight_semaphore = dispatch_semaphore_create(3);
    
    [self _setupMetal];
    [self _loadAssets];
    [self _reshape];
}

- (void)_setupMetal {
    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    
    _view = (MTKView *)self.view;
    _view.device = device;
    _view.delegate = self;
    
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
 
#pragma warning Delete this
    _renderContext = new VRORenderContextMetal(_view, device);
}

- (void)_loadAssets {
    _layerA = new VROLayer();
    _layerA->hydrate(*_renderContext);
    
    _layerB = new VROLayer();
    _layerB->hydrate(*_renderContext);
}

- (void)_render {
    dispatch_semaphore_wait(_inflight_semaphore, DISPATCH_TIME_FOREVER);

    /*
     A single command buffer collects all render events for frame.
     */
    id <MTLCommandBuffer> commandBuffer = [_renderContext->getCommandQueue() commandBuffer];
    commandBuffer.label = @"CommandBuffer";

    /*
     When the command buffer is executed by the GPU, signal the semaphor
     (required by the view since it will signal set up the next buffer).
     */
    __block dispatch_semaphore_t block_sema = _inflight_semaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(block_sema);
    }];
    
    // Obtain a renderPassDescriptor generated from the view's drawable textures
    MTLRenderPassDescriptor* renderPassDescriptor = _view.currentRenderPassDescriptor;
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:_view.currentRenderPassDescriptor];
    renderEncoder.label = @"ScreenRenderEncoder";

    _renderContext->setRenderPass(renderPassDescriptor);
    _renderContext->setCommandBuffer(commandBuffer);
    _renderContext->setRenderEncoder(renderEncoder);
    _renderContext->setConstantDataBufferIndex(_constantDataBufferIndex);

    if(renderPassDescriptor != nil) // If we have a valid drawable, begin the commands to render into it
    {
        _layerA->render(*_renderContext);
        _layerB->render(*_renderContext);
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:_view.currentDrawable];
    }

    // The render assumes it can now increment the buffer index and that the previous index won't be touched until we cycle back around to the same index
    _constantDataBufferIndex = (_constantDataBufferIndex + 1) % kMaxInflightBuffers;

    // Finalize rendering here & push the command buffer to the GPU
    [commandBuffer commit];
}

- (void)_reshape {
    // When reshape is called, update the view and projection matricies since this means the view orientation or size changed
    float aspect = fabs(self.view.bounds.size.width / self.view.bounds.size.height);
    _renderContext->setProjectionMatrix(matrix_from_perspective_fov_aspectLH(65.0f * (M_PI / 180.0f), aspect, 0.1f, 100.0f));
    _renderContext->setViewMatrix(matrix_identity_float4x4);
}

// Called whenever view changes orientation or layout is changed
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    [self _reshape];
}

// Called whenever the view needs to render
- (void)drawInMTKView:(nonnull MTKView *)view {
    @autoreleasepool {
        [self _render];
    }
}

@end

