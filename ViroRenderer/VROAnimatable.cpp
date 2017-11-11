//
//  VROAnimatable.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

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
