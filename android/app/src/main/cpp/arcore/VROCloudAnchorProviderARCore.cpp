//
//  VROCloudAnchorProviderARCore.h
//  ViroKit
//
//  Created by Raj Advani on 5/8/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROCloudAnchorProviderARCore.h"

VROCloudAnchorProviderARCore::VROCloudAnchorProviderARCore() {

}

VROCloudAnchorProviderARCore::~VROCloudAnchorProviderARCore() {

}

void VROCloudAnchorProviderARCore::hostAnchor(std::shared_ptr<VROARAnchor> anchor,
                                              arcore::Session *session,
                                              std::function<void(std::string anchorId)> onSuccess,
                                              std::function<void(std::string error)> onFailure) {


}

void VROCloudAnchorProviderARCore::resolveAnchor(std::string anchorId,
                                                 arcore::Session *session,
                                                 std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                                                 std::function<void(std::string error)> onFailure) {

}

void VROCloudAnchorProviderARCore::onFrameWillRender(const VRORenderContext &context);
void VROCloudAnchorProviderARCore::onFrameDidRender(const VRORenderContext &context);