//
//  VROIBLPreprocess.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROIBLPreprocess_h
#define VROIBLPreprocess_h

#include "VROPreprocess.h"

class VROTexture;

class VROIBLPreprocess : public VROPreprocess {
public:
    
    VROIBLPreprocess();
    virtual ~VROIBLPreprocess();
    
    virtual void execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                         std::shared_ptr<VRODriver> driver);
    
private:
    
    std::shared_ptr<VROTexture> _currentLightingEnvironment;
};

#endif /* VROIBLPreprocess_h */
