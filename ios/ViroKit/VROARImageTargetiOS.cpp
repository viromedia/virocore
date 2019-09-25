//
//  VROARImageTargetiOS.cpp
//  ViroKit
//
//  Created by Andy Chu on 1/30/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROARImageTargetiOS.h"
#include <ARKit/ARKit.h>

VROARImageTargetiOS::VROARImageTargetiOS(UIImage *sourceImage, VROImageOrientation orientation, float physicalWidth) :
    VROARImageTarget(orientation, physicalWidth),
    _sourceImage(sourceImage) {
        
}

VROARImageTargetiOS::~VROARImageTargetiOS() {
    
}

void VROARImageTargetiOS::initWithTrackingImpl(VROImageTrackingImpl impl) {
    _currentImpl = impl;
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    if (@available(iOS 11.3, *)) {
        if (_currentImpl == VROImageTrackingImpl::ARKit) {
            CGImagePropertyOrientation cgOrientation;
            switch (getOrientation()) {
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
                                                          physicalWidth:getPhysicalWidth()];
        }
    }
#endif
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
ARReferenceImage *VROARImageTargetiOS::getARReferenceImage() {
    return _referenceImage;
}
#endif
