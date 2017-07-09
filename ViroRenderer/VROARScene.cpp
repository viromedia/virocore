//
//  VROARScene.cpp
//  ViroKit
//
//  Created by Andy Chu on 6/13/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARScene.h"
#include "VROARAnchor.h"

void VROARScene::addNode(std::shared_ptr<VRONode> node) {
    VROScene::addNode(node);
}

void VROARScene::setARComponentManager(std::shared_ptr<VROARComponentManager> arComponentManager) {
    _arComponentManager = arComponentManager;
    std::vector<std::shared_ptr<VROARPlane>>::iterator it;
    for (it = _planes.begin(); it < _planes.end(); it++) {
        _arComponentManager->addARPlane(*it);
    }
}

void VROARScene::willAppear() {
    if (_arComponentManager) {
        std::vector<std::shared_ptr<VROARPlane>>::iterator it;
        for (it = _planes.begin(); it < _planes.end(); it++) {
            _arComponentManager->addARPlane(*it);
        }
    }
}

void VROARScene::willDisappear() {
    if (_arComponentManager) {
        _arComponentManager->clearAllPlanes(_planes);
    }
}

void VROARScene::addARPlane(std::shared_ptr<VROARPlane> plane) {
    // TODO: figure out a way to make ARNode simply not be visible at start.
    // call this once, because when it planes are first added they should not be visible.
    plane->setIsAttached(false);
    _planes.push_back(plane);
    if (_arComponentManager) {
        _arComponentManager->addARPlane(plane);
    }
}

void VROARScene::removeARPlane(std::shared_ptr<VROARPlane> plane) {
    plane->setIsAttached(false);
    if (_arComponentManager) {
        _arComponentManager->removeARPlane(plane);
    }
    _planes.erase(
                   std::remove_if(_planes.begin(), _planes.end(),
                                  [plane](std::shared_ptr<VROARPlane> candidate) {
                                      return candidate == plane;
                                  }), _planes.end());
}

void VROARScene::updateARPlane(std::shared_ptr<VROARPlane> plane) {
    if (_arComponentManager) {
        _arComponentManager->updateARPlane(plane);
    }
}
