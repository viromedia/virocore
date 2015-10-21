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
#import "VROScene.h"
#import "VROLayer.h"
#import "VROMath.h"
#import "VROImageUtil.h"

@implementation GameViewController
{
    // view
    MTKView *_view;
    
    VRORenderContextMetal *_renderContext;
    VROScene *_scene;
    
    dispatch_semaphore_t _inflight_semaphore;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
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
    _scene = new VROScene();
    
    std::shared_ptr<VROLayer> layerA = std::make_shared<VROLayer>();
    layerA->setFrame(VRORectMake(-1.0, 0, 2, 2.0, 2.0));
    layerA->setBackgroundColor({ 1.0, 0.0, 0.0, 1.0 });
    layerA->hydrate(*_renderContext);
    
    size_t dataLength;
    int width, height;
    void *data = VROImageLoadTextureDataRGBA8888("boba", &dataLength, &width, &height);
    
    layerA->setContents(data, dataLength);
    
    std::shared_ptr<VROLayer> layerB = std::make_shared<VROLayer>();
    layerB->setFrame(VRORectMake(1.0, 1.0, 1.0, 1.0));
    layerB->setBackgroundColor({ 0.0, 0.0, 1.0, 1.0 });
    layerB->hydrate(*_renderContext);
    
    layerB->setContents(data, dataLength);

    std::shared_ptr<VROLayer> layerC = std::make_shared<VROLayer>();
    layerC->setFrame(VRORectMake(0.0, 0.0, 0.5, 0.5));
    layerC->setBackgroundColor({ 0.0, 1.0, 0.0, 1.0 });
    layerC->hydrate(*_renderContext);
    
    layerC->setContents(data, dataLength);
    
    _scene->addLayer(layerA);
    layerA->addSublayer(layerB);
    layerB->addSublayer(layerC);
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

    if(renderPassDescriptor != nil) // If we have a valid drawable, begin the commands to render into it
    {
        _scene->render(*_renderContext);
                
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:_view.currentDrawable];
    }

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

