//
//  VRODriverCardboard.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VRODriverCardboard.h"
#import "VROViewport.h"
#import "VROFieldOfView.h"
#import "VRODriverContextMetal.h"
#import "VROCardboardViewDelegate.h"

VRODriverCardboard::VRODriverCardboard(std::shared_ptr<VRORenderer> renderer) :
    _renderer(renderer) {
        
    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    _context = std::make_shared<VRODriverContextMetal>(device);
    
    _delegate = [[VROCardboardViewDelegate alloc] init];
}

VRODriverCardboard::~VRODriverCardboard() {
    
}

UIView *VRODriverCardboard::getRenderingView() {
    if (!_view) {
        _view = [[GCSCardboardView alloc] initWithFrame:CGRectZero];
        _view.delegate = _delegate;
    }
    
    return _view;
}

void VRODriverCardboard::onOrientationChange(UIInterfaceOrientation orientation) {
    
}

VROViewport VRODriverCardboard::getViewport(VROEyeType eye) {
    return {};
}

VROFieldOfView VRODriverCardboard::getFOV(VROEyeType eye) {
    return {};
}
