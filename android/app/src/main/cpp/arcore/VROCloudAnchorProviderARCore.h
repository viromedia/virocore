//
//  VROCloudAnchorProviderARCore.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
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

    /*
     Convert anchor status to error messages.
     */
    std::string getAnchorStatusErrorMessage(arcore::AnchorAcquireStatus status);

};


#endif //ANDROID_VROCLOUDANCHORPROVIDERARCORE_H
