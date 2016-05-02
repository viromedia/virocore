//
//  VROScreenUIView.m
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROScreenUIView.h"
#import "VROEye.h"
#import "VROReticle.h"

// The size of the UIView we render to; this is scaled up to fill the VR HUD
static const int kUIViewSize = 300;

// The depth of the HUD, further away is more comfortable on the eyes but
// may create artifacts if objects are rendered *closer* than the HUD
static const float kVROLayerDepth = -1.5;

// The size of the VROLayer onto which the UIView is rendered
static const float kVROLayerSize = 2;

@interface VROScreenUIView () {
    std::shared_ptr<VROLayer> _layer;
    BOOL _needsUpdate;
}

@property (readwrite, nonatomic) BOOL reticleEnabled;
@property (readwrite, nonatomic) VROReticle *reticle;

@end

@implementation VROScreenUIView

- (instancetype)init {
    self = [super initWithFrame:CGRectMake(0, 0, kUIViewSize, kUIViewSize)];
    if (self) {
        /*
         Place the layer in front of the camera, far enough
         away to avoid discomfort.
         */
        _layer = std::make_shared<VROLayer>();
        _layer->setFrame(VRORectMake(-kVROLayerSize / 2.0, -kVROLayerSize / 2.0, kVROLayerDepth,
                                     kVROLayerSize, kVROLayerSize));
        
        std::shared_ptr<VROMaterial> material = _layer->getMaterial();
        material->setReadsFromDepthBuffer(false);
        material->setWritesToDepthBuffer(false);
        material->setLightingModel(VROLightingModel::Constant);
        
        [self setBackgroundColor:[UIColor clearColor]];
        self.reticle = [[VROReticle alloc] init];
        _needsUpdate = YES;
    }
    
    return self;
}

- (void)dealloc {
    
}

- (void)updateWithContext:(const VRODriver *)context {
    if (!_needsUpdate) {
        return;
    }
    _needsUpdate = NO;
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    CGFloat scale = [[UIScreen mainScreen] nativeScale];
    int width  = self.bounds.size.width  * scale;
    int height = self.bounds.size.height * scale;
    
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;
    
    CGContextRef bitmapContext = CGBitmapContextCreate(nullptr, width, height,
                                                       bitsPerComponent, bytesPerRow, colorSpace,
                                                       kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    
    // Flip since we'll be rendering to a texture
    CGAffineTransform flipVertical = CGAffineTransformMake(1, 0, 0, -1, 0, height);
    CGContextConcatCTM(bitmapContext, flipVertical);
    CGContextScaleCTM(bitmapContext, scale, scale);
    
    [self.layer renderInContext:bitmapContext];
    if (self.reticleEnabled) {
        [self.reticle renderRect:self.bounds context:bitmapContext];
    }
    _layer->setContents(width, height, bitmapContext, *context);
    
    CGContextRelease(bitmapContext);
}

- (void)renderEye:(VROEyeType)eye
withRenderContext:(const VRORenderContext *)renderContext
    driver:(const VRODriver *)driver {
    
    VRORenderParameters renderParams;
    renderParams.transforms.push(renderContext->getHUDViewMatrix());
    renderParams.opacities.push(1.0);
    
    _layer->render(*renderContext, *driver, renderParams);
}

- (void)setReticleEnabled:(BOOL)enabled {
    if (!_reticleEnabled && enabled) {
        [self addSubview:self.reticle];
    }
    else if (_reticleEnabled && !enabled) {
        [self.reticle removeFromSuperview];
    }
    
    _reticleEnabled = enabled;
    [self setNeedsUpdate];
}

- (void)setDepth:(float)depth {
    _layer->setPosition({_layer->getPosition().x, _layer->getPosition().y, depth});
    [self setNeedsUpdate];
}

- (void)setNeedsUpdate {
    _needsUpdate = YES;
}

@end