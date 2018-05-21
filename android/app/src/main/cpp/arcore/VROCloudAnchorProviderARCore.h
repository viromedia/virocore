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
class VROARAnchorARCore;
class VROARSessionARCore;

class VROCloudAnchorHostTask {
public:
    std::shared_ptr<VROARAnchor> originalAnchor;
    std::shared_ptr<VROARAnchorARCore> cloudAnchor;
    std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess;
    std::function<void(std::string error)> onFailure;
};

class VROCloudAnchorResolveTask {
public:
    std::string cloudAnchorId;
    std::shared_ptr<VROARAnchorARCore> cloudAnchor;
    std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess;
    std::function<void(std::string error)> onFailure;
};

/*
 Manages the hosting and resolution of cloud anchors from ARCore.
 */
class VROCloudAnchorProviderARCore : public VROFrameListener {

public:

    VROCloudAnchorProviderARCore(std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROCloudAnchorProviderARCore();

    /*
     Host an anchor on the cloud anchor provider we're using. Hosting an anchor is an
     asynchronous process that will eventually return the hosted cloud anchor to the
     given callback.
     */
    void hostCloudAnchor(std::shared_ptr<VROARAnchor> anchor,
                         std::function<void(std::shared_ptr<VROARAnchor>)> onSuccess,
                         std::function<void(std::string error)> onFailure);

    /*
     Resolve an anchor with the given ID from the cloud anchor. This is an
     asynchronous process. If found, the anchor will be returned in the given
     callback.
     */
    void resolveCloudAnchor(std::string cloudAnchorId,
                            std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                            std::function<void(std::string error)> onFailure);

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:

    std::weak_ptr<VROARSessionARCore> _session;

    /*
     Vector of cloud anchors that we are attempting to host.
     */
    std::vector<VROCloudAnchorHostTask> _queuedHosting;

    /*
     Vector of cloud anchors that we are attempting to resolve.
     */
    std::vector<VROCloudAnchorResolveTask> _queuedResolving;

    /*
     Retrieve an error string from an ARCore error code.
     */
    std::string getError(arcore::CloudAnchorState state);

    /*
     Handle successful / failed hosting.
     */
    void onHostTaskSuccessful(VROCloudAnchorHostTask &task);
    void onHostTaskFailed(VROCloudAnchorHostTask &task, std::string error);

    /*
     Handle successful / failed resolving.
     */
    void onResolveTaskSuccessful(VROCloudAnchorResolveTask &task);
    void onResolveTaskFailed(VROCloudAnchorResolveTask &task, std::string error);

};


#endif //ANDROID_VROCLOUDANCHORPROVIDERARCORE_H
