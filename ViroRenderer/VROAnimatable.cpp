//
//  VROAnimatable.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#include "VROAnimatable.h"
#include "VROTransaction.h"
#include "VROAnimation.h"
#include "VROThreadRestricted.h"
#include "VROLog.h"

void VROAnimatable::animate(std::shared_ptr<VROAnimation> animation) {
    animation->setAnimatable(shared_from_this());

    // If we're not on the rendering thread, or if there is no current transaction,
    // then do not animate: skip straight to the final value.
    if (VROThreadRestricted::isThread(VROThreadName::Renderer)) {
        std::shared_ptr<VROTransaction> transaction = VROTransaction::get();
        if (!transaction || transaction->isDegenerate()) {
            animation->onTermination();
        } else {
            transaction->addAnimation(animation);
        }
    }
    else {
        animation->onTermination();
    }
}
