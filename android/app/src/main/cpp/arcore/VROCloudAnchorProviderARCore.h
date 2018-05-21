//
//  VROCloudAnchorProviderARCore.h
//  ViroKit
//
//  Created by Raj Advani on 5/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROCLOUDANCHORPROVIDERARCORE_H
#define ANDROID_VROCLOUDANCHORPROVIDERARCORE_H

#include <vector>
#include <string>
#include <memory>
#include "VROFrameListener.h"
#include "ARCore_API.h"

class VROARAnchor;

/*
 Manages the hosting and resolution of cloud anchors from ARCore.
 */
class VROCloudAnchorProviderARCore : public VROFrameListener {

public:

    VROCloudAnchorProviderARCore();
    virtual ~VROCloudAnchorProviderARCore();

    /*
     Host an anchor on the cloud anchor provider we're using. Hosting an anchor is an
     asynchronous process that will eventually return an anchor ID to the
     given callback.
     */
    void hostAnchor(std::shared_ptr<VROARAnchor> anchor,
                    arcore::Session *session,
                    std::function<void(std::string anchorId)> onSuccess,
                    std::function<void(std::string error)> onFailure);

    /*
     Resolve an anchor with the given ID from the cloud anchor. This is an
     asynchronous process. If found, the anchor will be returned in the given
     callback.
     */
    void resolveAnchor(std::string anchorId,
                       arcore::Session *session,
                       std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                       std::function<void(std::string error)> onFailure);

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:

    /*
     Vector of cloud anchors that we are attempting to host.
     */
    std::vector<std::shared_ptr<VROARAnchor>> _queuedHosting;

    /*
     Vector of cloud anchors that we are attempting to resolve.
     */
    std::vector<std::shared_ptr<VROARAnchor>> _queuedResolving;

};


#endif //ANDROID_VROCLOUDANCHORPROVIDERARCORE_H
