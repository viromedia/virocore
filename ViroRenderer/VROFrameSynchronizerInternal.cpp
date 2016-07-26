//
//  VROFrameSynchronizerInternal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROFrameSynchronizerInternal.h"
#include "VROFrameListener.h"

void VROFrameSynchronizerInternal::addFrameListener(std::shared_ptr<VROFrameListener> listener) {
    _frameListeners.push_back(listener);
}

void VROFrameSynchronizerInternal::removeFrameListener(std::shared_ptr<VROFrameListener> listener) {
    _frameListeners.erase(
                          std::remove_if(_frameListeners.begin(), _frameListeners.end(),
                                         [this, listener](std::weak_ptr<VROFrameListener> l) {
                                             std::shared_ptr<VROFrameListener> locked = l.lock();
                                             return locked && locked == listener;
                                         }), _frameListeners.end());
}

void VROFrameSynchronizerInternal::notifyFrameStart(const VRORenderContext &context) {
    auto it = _frameListeners.begin();
    
    while (it != _frameListeners.end()) {
        std::weak_ptr<VROFrameListener> listener = *it;
        std::shared_ptr<VROFrameListener> locked = listener.lock();
        
        if (locked) {
            locked->onFrameWillRender(context);
            ++it;
        }
        else {
            it = _frameListeners.erase(it);
        }
    }
}

void VROFrameSynchronizerInternal::notifyFrameEnd(const VRORenderContext &context) {
    auto it = _frameListeners.begin();
    
    while (it != _frameListeners.end()) {
        std::weak_ptr<VROFrameListener> listener = *it;
        std::shared_ptr<VROFrameListener> locked = listener.lock();
        
        if (locked) {
            locked->onFrameDidRender(context);
            ++it;
        }
        else {
            it = _frameListeners.erase(it);
        }
    }
}