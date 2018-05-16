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
    VROInputControllerCardboard(std::shared_ptr<VRODriver> driver) : VROInputControllerBase(driver) {}
    virtual ~VROInputControllerCardboard(){}

    virtual VROVector3f getDragForwardOffset();

    virtual void onProcess(const VROCamera &camera);

    virtual std::string getHeadset() {
        return "cardboard";
    }

    virtual std::string getController() {
        return "cardboard";
    }

    void updateScreenTouch(int touchAction);

protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRODriver> driver) {
        return std::make_shared<VROInputPresenterCardboard>();
    }

private:
    void updateOrientation(const VROCamera &camera);
};
#endif
