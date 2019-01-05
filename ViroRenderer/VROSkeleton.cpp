//
//  VROSkeleton.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSkeleton.h"
#include "VROBone.h"
#include "VROStringUtil.h"

VROSkeleton::VROSkeleton(std::vector<std::shared_ptr<VROBone>> bones) {
    _bones = bones;

    for (auto &bone : bones) {
        std::string boneName = bone->getName();
        if (VROStringUtil::trim(boneName).size() == 0) {
            continue;
        }

        _nameToBonesMap[boneName] = bone;
    }
}
