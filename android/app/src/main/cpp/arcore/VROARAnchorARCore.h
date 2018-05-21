//
//  VROARAnchorARCore.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARAnchorARCore_h
#define VROARAnchorARCore_h

#include "VROARAnchor.h"
#include "ARCore_API.h"

class VROARSessionARCore;

/*
 Superclass for all ARCore anchors. Unlike ARKit, where trackables and anchors are a merged
 concept, in ARCore they are separate. In ARCore, we check for updated trackables each frame
 in the VROARSessionARCore. For each trackable found, we create an ARAnchor corresponding to it:
 these are known as "managed" anchors. Managed anchors own the actual internal (ARCore) anchor,
 and maintain a link to the corresponding trackable (which is *also* a subclass of VRARAnchor).
 */
class VROARAnchorARCore : public VROARAnchor, public std::enable_shared_from_this<VROARAnchorARCore> {
public:
    
    VROARAnchorARCore(std::string key,
                      std::shared_ptr<arcore::Anchor> anchor,
                      std::shared_ptr<VROARAnchor> trackable,
                      std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARAnchorARCore();

    /*
     Managed anchors are anchors that are automatically created by Viro and assigned to each
     detected trackable.
     */
    bool isManaged() const {
        return _trackable != nullptr;
    }

    /*
     Sync this anchor with the latest data in its underlying ARCore anchor.
     */
    void sync();

    /*
     Detach this anchor from the session, so that it no longer receives updates. This will
     also remove the corresponding ARNode from the scene.

     Invoke from rendering thread.
     */
    void detach();

    /*
     Get the cloud identifier for this anchor. This is only valid for anchors that have been
     hosted to the cloud or resolved from the cloud.
     */
    std::string getCloudAnchorId() const;
    void loadCloudAnchorId();

    std::shared_ptr<VROARAnchor> getTrackable();
    std::shared_ptr<arcore::Anchor> getAnchorInternal();

private:

    std::shared_ptr<arcore::Anchor> _anchor;
    std::shared_ptr<VROARAnchor> _trackable;
    std::weak_ptr<VROARSessionARCore> _session;
    std::string _cloudAnchorId;

};

#endif /* VROARAnchorARCore_h */
