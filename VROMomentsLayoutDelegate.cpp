//
//  VROMomentsLayoutDelegate.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/16/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMomentsLayoutDelegate.h"

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getCenterLayer() {
    return centerLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getTopLayer() {
    return topLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getBottomLayer() {
    return bottomLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getLeftLayer() {
    return leftLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getRightLayer() {
    return rightLayer;
}