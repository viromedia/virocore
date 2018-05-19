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

/*
 Superclass for all ARCore anchors. Unlike ARKit, where trackables and anchors are a merged
 concept, in ARCore they are separate. In ARCore, we check for updated trackables each frame
 in the VROARSessionARCore. For each trackable found, we create an ARAnchor corresponding to it.
 This object owns the actual internal (ARCore) anchor, and maintains a link to the corresponding
 trackable (which is *also* a subclass of VRARAnchor).
 */
class VROARAnchorARCore : public VROARAnchor {
public:
    
    VROARAnchorARCore(std::string key,
                      std::shared_ptr<arcore::Anchor> anchor,
                      std::shared_ptr<VROARAnchor> trackable);
    virtual ~VROARAnchorARCore();

    std::shared_ptr<VROARAnchor> getTrackable();
    std::shared_ptr<arcore::Anchor> getAnchorInternal();

private:

    std::shared_ptr<arcore::Anchor> _anchor;
    std::shared_ptr<VROARAnchor> _trackable;

};

#endif /* VROARAnchorARCore_h */
