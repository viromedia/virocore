//
//  VROFrustumBoxIntersectionMetadata.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/17/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROFrustumBoxIntersectionMetadata.h"

VROFrustumBoxIntersectionMetadata::VROFrustumBoxIntersectionMetadata() :
    _sourceFrustumForDistances(nullptr),
    _distanceFrame(UINT32_MAX),
    _planeLastOutside(0),
    _distancesValid(false) {
    
}

VROFrustumBoxIntersectionMetadata::~VROFrustumBoxIntersectionMetadata() {
    
}

void VROFrustumBoxIntersectionMetadata::reset() {
    _distanceFrame = UINT32_MAX;
    _sourceFrustumForDistances = nullptr;
    _distancesValid = false;
}
