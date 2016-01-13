//
//  VROScreenUIView.m
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROScreenUIView.h"
#import "VROEye.h"

// The size of the UIView we render to; this is scaled up to fill the VR HUD
static const int kUIViewSize = 300;

// The depth of the HUD, further away is more comfortable on the eyes but
// may create artifacts if objects are rendered *closer* than the HUD
static const float kVROLayerDepth = -1.5;

// The size of the VROLayer onto which the UIView is rendered
static const float kVROLayerSize = 2;

@interface VROScreenUIView () {
    std::shared_ptr<VROLayer> _layer;
}

@end

@implementation VROScreenUIView

- (instancetype)initWithContext:(VRORenderContext *)context {
    self = [super initWithFrame:CGRectMake(0, 0, kUIViewSize, kUIViewSize)];
    if (self) {
        /*
         Place the layer in front of the camera, far enough
         away to avoid discomfort.
         */
        _layer = std::make_shared<VROLayer>(*context);
        _layer->setFrame(VRORectMake(-kVROLayerSize / 2.0, -kVROLayerSize / 2.0, kVROLayerDepth,
                                     kVROLayerSize, kVROLayerSize));
        
        std::shared_ptr<VROMaterial> material = _layer->getMaterial();
        material->setReadsFromDepthBuffer(false);
        material->setWritesToDepthBuffer(false);
        
        [self setBackgroundColor:[UIColor clearColor]];
    }
    
    return self;
}

- (void)dealloc {
    
}

- (void)update {
    CGFloat scale = [UIScreen mainScreen].nativeScale;
    UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, scale);
    
    [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:YES];
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    _layer->setContents(image);
    
    UIGraphicsEndImageContext();
}

- (void)renderEye:(VROEye *)eye withContext:(VRORenderContext *)context {
    VROMatrix4f viewInversion = eye->getEyeView().invert();

    // Keep the HUD in front of the camera
    VRORenderParameters renderParams;
    renderParams.rotations.push(viewInversion);
    renderParams.transforms.push(viewInversion);
    
    _layer->render(*context, renderParams);
}

- (std::shared_ptr<VROLayer>) vroLayer {
    return _layer;
}

@end