//
//  VROCloudAnchorProviderARCore.h
//  ViroKit
//
//  Created by Raj Advani on 5/8/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROCloudAnchorProviderARCore.h"
#include "VROARSessionARCore.h"
#include "VROARAnchorARCore.h"

VROCloudAnchorProviderARCore::VROCloudAnchorProviderARCore(std::shared_ptr<VROARSessionARCore> session) :
    _session(session) {

}

VROCloudAnchorProviderARCore::~VROCloudAnchorProviderARCore() {

}

void VROCloudAnchorProviderARCore::hostCloudAnchor(std::shared_ptr<VROARAnchor> anchor,
                                                   std::function<void(std::shared_ptr<VROARAnchor>)> onSuccess,
                                                   std::function<void(std::string error)> onFailure) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }
    arcore::Session *session_arc = session->getSessionInternal();

    std::shared_ptr<VROARAnchorARCore> anchor_v = std::dynamic_pointer_cast<VROARAnchorARCore>(anchor);
    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(
            session_arc->hostAndAcquireNewCloudAnchor(anchor_v->getAnchorInternal().get()));

    std::string key = VROStringUtil::toString64(anchor_arc->getId());
    std::shared_ptr<VROARAnchorARCore> cloudAnchor = std::make_shared<VROARAnchorARCore>(key, anchor_arc, nullptr, session);

    VROCloudAnchorHostTask task;
    task.originalAnchor = anchor;
    task.cloudAnchor = cloudAnchor;
    task.onSuccess = onSuccess;
    task.onFailure = onFailure;

    _queuedHosting.push_back(task);
}

void VROCloudAnchorProviderARCore::resolveCloudAnchor(std::string anchorId,
                                                      std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                                                      std::function<void(std::string error)> onFailure) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }
}

void VROCloudAnchorProviderARCore::onHostTaskSuccessful(VROCloudAnchorHostTask &task) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    // Transfer the old anchor's ARNode to the cloudAnchor, then remove the old anchor
    std::shared_ptr<VROARNode> node = task.originalAnchor->getARNode();
    task.cloudAnchor->sync();
    task.cloudAnchor->setARNode(node);

    session->removeAnchor(task.originalAnchor);
    session->addAnchor(task.cloudAnchor);

    task.onSuccess(task.cloudAnchor);
}

void VROCloudAnchorProviderARCore::onHostTaskFailed(VROCloudAnchorHostTask &task, std::string error) {
    task.onFailure(error);
}

void VROCloudAnchorProviderARCore::onFrameWillRender(const VRORenderContext &context) {
    for (auto it = _queuedHosting.begin(); it != _queuedHosting.end(); ) {
        VROCloudAnchorHostTask &hostTask = *it;
        std::shared_ptr<arcore::Anchor> anchor_arc = hostTask.cloudAnchor->getAnchorInternal();
        arcore::CloudAnchorState state = anchor_arc->getCloudAnchorState();

        bool removeFromTaskList = false;
        switch (state) {
            case arcore::CloudAnchorState::TaskInProgress:
                ++it;
                break;

            case arcore::CloudAnchorState::Success:
                onHostTaskSuccessful(hostTask);
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorInternal:
                onHostTaskFailed(hostTask, "Error hosting cloud error [Internal Error]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::None:
                onHostTaskFailed(hostTask, "Error hosting cloud error [No Error]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorNotAuthorized:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [Not Authorized]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorServiceUnavailable:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [Service Unavailable]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorResourceExhausted:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [Resource Exhausted]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorDatasetProcessingFailed:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [Dataset Processing Failed]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorCloudIDNotFound:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [Cloud ID Not Found]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorResolvingLocalizationNoMatch:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [Localization No Match]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooOld:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [SDK Version Too Old]");
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooNew:
                onHostTaskFailed(hostTask, "Error hosting cloud anchor [SDK Version Too New]");
                removeFromTaskList = true;
                break;
        }

        if (removeFromTaskList) {
            it = _queuedHosting.erase(it);
        }
    }

}

void VROCloudAnchorProviderARCore::onFrameDidRender(const VRORenderContext &context) {

}