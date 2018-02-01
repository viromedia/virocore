//
//  VROARImageTargetiOS.cpp
//  ViroKit
//
//  Created by Andy Chu on 1/30/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROARImageTargetiOS.h"
#include <ARKit/ARKit.h>

VROARImageTargetiOS::VROARImageTargetiOS(UIImage *sourceImage, VROImageOrientation orientation, float physicalWidth) :
    _sourceImage(sourceImage),
    _orientation(orientation),
    _physicalWidth(physicalWidth) {
        
}

VROARImageTargetiOS::~VROARImageTargetiOS() {
    
}

void VROARImageTargetiOS::initWithTrackingImpl(VROImageTrackingImpl impl) {
    _currentImpl = impl;
    if (@available(iOS 11.3, *)) {
        if (_currentImpl == VROImageTrackingImpl::ARKit) {
            CGImagePropertyOrientation cgOrientation;
            switch (_orientation) {
                case VROImageOrientation::Down:
                    cgOrientation = kCGImagePropertyOrientationDown;
                    break;
                case VROImageOrientation::Left:
                    cgOrientation = kCGImagePropertyOrientationLeft;
                    break;
                case VROImageOrientation::Right:
                    cgOrientation = kCGImagePropertyOrientationRight;
                    break;
                case VROImageOrientation::Up:
                default:
                    cgOrientation = kCGImagePropertyOrientationUp;
                    break;
            }
            _referenceImage = [[ARReferenceImage alloc] initWithCGImage:_sourceImage.CGImage
                                                            orientation:cgOrientation
                                                          physicalWidth:_physicalWidth];
        }
    }
}

ARReferenceImage *VROARImageTargetiOS::getARReferenceImage() {
    return _referenceImage;
}
