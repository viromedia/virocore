//
//  VROCrossLayout.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/11/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROCrossLayout.h"

VROCrossLayout::VROCrossLayout(std::shared_ptr<VROScene> scene) :
    _scene(scene) {
    
}

VROCrossLayout::~VROCrossLayout() {
    
}

void VROCrossLayout::layout() {
    std::shared_ptr<VROCrossLayoutDelegate> delegate = _delegate.lock();
    if (!delegate) {
        return;
    }
    
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (!scene) {
        return;
    }
    
    std::shared_ptr<VROLayer> centerLayer = delegate->getCenterLayer();
    std::shared_ptr<VROLayer> topLayer    = delegate->getTopLayer();
    std::shared_ptr<VROLayer> bottomLayer = delegate->getBottomLayer();
    std::shared_ptr<VROLayer> leftLayer   = delegate->getLeftLayer();
    std::shared_ptr<VROLayer> rightLayer  = delegate->getRightLayer();
    
    float size = 2;
    float padding = 1;
    float z = 4;
    
    centerLayer->setFrame(VRORectMake(-size / 2,             -size / 2,             z, size, size));
    topLayer->   setFrame(VRORectMake(-size / 2,              size / 2,             z, size, size));
    bottomLayer->setFrame(VRORectMake(-size / 2,             -size * 3/2 - padding, z, size, size));
    leftLayer->  setFrame(VRORectMake(-size * 3/2 - padding, -size / 2,             z, size, size));
    rightLayer-> setFrame(VRORectMake( size / 2 + padding,   -size / 2,             z, size, size));
    
    /*
    scene->addNode(centerLayer);
    scene->addNode(topLayer);
    scene->addNode(bottomLayer);
    scene->addNode(leftLayer);
    scene->addNode(rightLayer);
     */
}