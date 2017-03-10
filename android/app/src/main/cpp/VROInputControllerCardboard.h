//
//  VROInputControllerCardboard.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputControllerCardboard_H
#define VROInputControllerCardboard_H
#include <memory>
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROInputPresenterCardboard.h"
#include <android/input.h>

class VROInputControllerCardboard : public VROInputControllerBase {

public:
    VROInputControllerCardboard(){}
    virtual ~VROInputControllerCardboard(){}

    virtual void onProcess(const VROCamera &camera);

    virtual std::string getHeadset() {
        return "cardboard";
    }

    virtual std::string getController() {
        return "cardboard";
    }

    void updateScreenTouch(int touchAction);

protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRORenderContext> context){
        return std::make_shared<VROInputPresenterCardboard>(context);
    }

private:
    void updateOrientation(const VROCamera &camera);
};
#endif
