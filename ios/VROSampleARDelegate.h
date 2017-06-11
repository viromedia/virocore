//
//  VROSampleARDelegate.h
//  ViroRenderer
//
//  Created by Raj Advani on 6/11/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROSampleARDelegate_h
#define VROSampleARDelegate_h

#include <ViroKit/ViroKit.h>

class VROSampleARDelegate : public VROARSessionDelegate {
public:
    
    std::shared_ptr<VRONode> anchorWasDetected(std::shared_ptr<VROARAnchor> anchor);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor);
    
private:
    
    std::shared_ptr<VRONode> loadFBXNode(VROVector3f center);
    void animateTake(std::shared_ptr<VRONode> node);
    
};

#endif /* VROSampleARDelegate_h */
