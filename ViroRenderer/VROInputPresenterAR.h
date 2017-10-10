//
//  VROInputPresenterAR.h
//  ViroKit
//
//  Created by Andy Chu on 6/24/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterAR_h
#define VROInputPresenterAR_h

#include "VROInputPresenter.h"
#include <memory>

class VROInputPresenterAR : public VROInputPresenter {

public:
    VROInputPresenterAR() {
        setReticle(nullptr);
    }
    ~VROInputPresenterAR() {};
    
};
#endif /* VROInputPresenterAR_h */
