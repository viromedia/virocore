//
//  VROInputPresenterARiOS.h
//  ViroKit
//
//  Created by Andy Chu on 6/24/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterARiOS_h
#define VROInputPresenterARiOS_h

#include "VROInputPresenter.h"
#include <memory>

class VROInputPresenterARiOS : public VROInputPresenter {

public:
    VROInputPresenterARiOS() {
        setReticle(nullptr);
    }
    ~VROInputPresenterARiOS() {};
    
};
#endif /* VROInputPresenterARiOS_h */
