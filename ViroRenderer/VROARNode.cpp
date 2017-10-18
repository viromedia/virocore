//
//  VROARNode.cpp
//  ViroKit
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARAnchor.h"

void VROARNode::setPauseUpdates(bool pauseUpdates) {
    _pauseUpdates = pauseUpdates;
    if (!_pauseUpdates) {
        std::shared_ptr<VROARAnchor> anchor = _anchor.lock();
        if (anchor) {
            anchor->updateNodeTransform();
        }
    }
}
