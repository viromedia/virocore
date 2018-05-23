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

    arcore::AnchorAcquireStatus status;
    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(
            session_arc->hostAndAcquireNewCloudAnchor(anchor_v->getAnchorInternal().get(), &status));
    if (!anchor_arc) {
        // ARCore can immediately fail to host a cloud anchor for a number of reasons
        onFailure("Failed to host cloud anchor [error: " + getAnchorStatusErrorMessage(status) + "]");
        return;
    }

    std::string key = VROStringUtil::toString64(anchor_arc->getId());
    std::shared_ptr<VROARAnchorARCore> cloudAnchor = std::make_shared<VROARAnchorARCore>(key, anchor_arc, nullptr, session);

    VROCloudAnchorHostTask task;
    task.originalAnchor = anchor;
    task.cloudAnchor = cloudAnchor;
    task.onSuccess = onSuccess;
    task.onFailure = onFailure;

    _queuedHosting.push_back(task);
}

void VROCloudAnchorProviderARCore::onHostTaskSuccessful(VROCloudAnchorHostTask &task) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    // Transfer the old anchor's ARNode to the cloudAnchor, then remove the old anchor
    std::shared_ptr<VROARNode> node = task.originalAnchor->getARNode();
    task.cloudAnchor->sync();
    task.cloudAnchor->loadCloudAnchorId();
    task.cloudAnchor->setARNode(node);

    session->removeAnchor(task.originalAnchor);
    session->addAnchor(task.cloudAnchor);

    task.onSuccess(task.cloudAnchor);
}

void VROCloudAnchorProviderARCore::onHostTaskFailed(VROCloudAnchorHostTask &task, std::string error) {
    task.onFailure(error);
}

void VROCloudAnchorProviderARCore::resolveCloudAnchor(std::string cloudAnchorId,
                                                      std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                                                      std::function<void(std::string error)> onFailure) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }
    arcore::Session *session_arc = session->getSessionInternal();

    arcore::AnchorAcquireStatus status;
    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(
            session_arc->resolveAndAcquireNewCloudAnchor(cloudAnchorId.c_str(), &status));
    if (!anchor_arc) {
        // ARCore can immediately fail to resolve a cloud anchor for a number of reasons
        onFailure("Failed to resolve cloud anchor [error: " + getAnchorStatusErrorMessage(status) + "]");
        return;
    }

    std::string key = VROStringUtil::toString64(anchor_arc->getId());
    std::shared_ptr<VROARAnchorARCore> cloudAnchor = std::make_shared<VROARAnchorARCore>(key, anchor_arc, nullptr, session);

    VROCloudAnchorResolveTask task;
    task.cloudAnchorId = cloudAnchorId;
    task.cloudAnchor = cloudAnchor;
    task.onSuccess = onSuccess;
    task.onFailure = onFailure;

    _queuedResolving.push_back(task);
}

void VROCloudAnchorProviderARCore::onResolveTaskSuccessful(VROCloudAnchorResolveTask &task) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    task.cloudAnchor->sync();
    task.cloudAnchor->loadCloudAnchorId();
    session->addAnchor(task.cloudAnchor);

    task.onSuccess(task.cloudAnchor);
}

void VROCloudAnchorProviderARCore::onResolveTaskFailed(VROCloudAnchorResolveTask &task, std::string error) {
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

            case arcore::CloudAnchorState::None:
            case arcore::CloudAnchorState::ErrorInternal:
            case arcore::CloudAnchorState::ErrorNotAuthorized:
            case arcore::CloudAnchorState::ErrorServiceUnavailable:
            case arcore::CloudAnchorState::ErrorResourceExhausted:
            case arcore::CloudAnchorState::ErrorDatasetProcessingFailed:
            case arcore::CloudAnchorState::ErrorCloudIDNotFound:
            case arcore::CloudAnchorState::ErrorResolvingLocalizationNoMatch:
            case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooOld:
            case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooNew:
                onHostTaskFailed(hostTask, getError(state));
                removeFromTaskList = true;
                break;
        }

        if (removeFromTaskList) {
            it = _queuedHosting.erase(it);
        }
    }

    for (auto it = _queuedResolving.begin(); it != _queuedResolving.end(); ) {
        VROCloudAnchorResolveTask &resolveTask = *it;
        std::shared_ptr<arcore::Anchor> anchor_arc = resolveTask.cloudAnchor->getAnchorInternal();
        arcore::CloudAnchorState state = anchor_arc->getCloudAnchorState();

        bool removeFromTaskList = false;
        switch (state) {
            case arcore::CloudAnchorState::TaskInProgress:
                ++it;
                break;

            case arcore::CloudAnchorState::Success:
                onResolveTaskSuccessful(resolveTask);
                removeFromTaskList = true;
                break;

            case arcore::CloudAnchorState::None:
            case arcore::CloudAnchorState::ErrorInternal:
            case arcore::CloudAnchorState::ErrorNotAuthorized:
            case arcore::CloudAnchorState::ErrorServiceUnavailable:
            case arcore::CloudAnchorState::ErrorResourceExhausted:
            case arcore::CloudAnchorState::ErrorDatasetProcessingFailed:
            case arcore::CloudAnchorState::ErrorCloudIDNotFound:
            case arcore::CloudAnchorState::ErrorResolvingLocalizationNoMatch:
            case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooOld:
            case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooNew:
                onResolveTaskFailed(resolveTask, getError(state));
                removeFromTaskList = true;
                break;
        }

        if (removeFromTaskList) {
            it = _queuedResolving.erase(it);
        }
    }
}

void VROCloudAnchorProviderARCore::onFrameDidRender(const VRORenderContext &context) {

}

std::string VROCloudAnchorProviderARCore::getError(arcore::CloudAnchorState state) {
    switch (state) {
        case arcore::CloudAnchorState::ErrorInternal:
            return "Internal Error";
        case arcore::CloudAnchorState::None:
            return "No Error";
        case arcore::CloudAnchorState::ErrorNotAuthorized:
            return "Not Authorized";
        case arcore::CloudAnchorState::ErrorServiceUnavailable:
            return "Service Unavailable";
        case arcore::CloudAnchorState::ErrorResourceExhausted:
            return "Resource Exhausted";
        case arcore::CloudAnchorState::ErrorDatasetProcessingFailed:
            return "Dataset Processing Failed";
        case arcore::CloudAnchorState::ErrorCloudIDNotFound:
            return "Cloud ID Not Found";
        case arcore::CloudAnchorState::ErrorResolvingLocalizationNoMatch:
            return "Localization No Match";
        case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooOld:
            return "SDK Version Too Old";
        case arcore::CloudAnchorState::ErrorResolvingSDKVersionTooNew:
            return "SDK Version Too New";
        default:
            return "Unknown Error";
    }
}

std::string VROCloudAnchorProviderARCore::getAnchorStatusErrorMessage(arcore::AnchorAcquireStatus status) {
    switch (status) {
        case arcore::AnchorAcquireStatus::ErrorUnknown:
            return "Unknown error";
        case arcore::AnchorAcquireStatus::ErrorNotTracking:
            return "Tracking limited or not tracking, try again in better conditions";
        case arcore::AnchorAcquireStatus::ErrorAnchorNotSupportedForHosting:
            return "Anchor is not supported for hosting";
        case arcore::AnchorAcquireStatus::ErrorCloudAnchorsNotConfigured:
            return "Cloud anchors are not configured";
        case arcore::AnchorAcquireStatus::ErrorDeadlineExceeded:
            return "Deadline exceeded to create anchor";
        case arcore::AnchorAcquireStatus::ErrorResourceExhausted:
            return "Resources exhausted: too many anchors";
        case arcore::AnchorAcquireStatus::ErrorSessionPaused:
            return "Session paused";
        case arcore::AnchorAcquireStatus::Success:
            return "Success";
        default:
            return "Unknown error";
    }
}
