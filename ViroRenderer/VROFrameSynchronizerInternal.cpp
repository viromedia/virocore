//
//  VROFrameSynchronizerInternal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#include "VROFrameSynchronizerInternal.h"
#include "VROFrameListener.h"
#include <algorithm>

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
