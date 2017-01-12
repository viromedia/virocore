//
//  VROControllerInputCardboardiOS.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROControllerInputCardboardiOS_H
#define VROControllerInputCardboardiOS_H

#include <memory>
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROInputPresenterCardboardiOS.h"

class VROInputControllerCardboardiOS : public VROInputControllerBase {

public:
    VROInputControllerCardboardiOS(){}
    virtual ~VROInputControllerCardboardiOS(){}

    virtual void onProcess();
    void onScreenClicked();

protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRORenderContext> context){
        return std::make_shared<VROInputPresenterCardboardiOS>(context);
    }

private:
    void updateOrientation();
};
#endif
