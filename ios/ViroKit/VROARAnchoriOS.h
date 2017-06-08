//
//  VROARAnchoriOS.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARAnchoriOS_h
#define VROARAnchoriOS_h

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARAnchor.h"
#include <ARKit/ARKit.h>

class VROARAnchoriOS : public VROARAnchor {
public:
    
    VROARAnchoriOS(ARAnchor *anchor);
    virtual ~VROARAnchoriOS();
    
    VROMatrix4f getTransform() const;
    
private:
    
    ARAnchor *_anchor;
    
};

#endif
#endif /* VROARAnchoriOS_h */
