//
//  VROInputControllerWasm.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROInputControllerWasm_H
#define VROInputControllerWasm_H

#include <memory>
#include "VRORenderContext.h"
#include "VROInputControllerBase.h"
#include "VROInputPresenterWasm.h"

class VROInputControllerWasm : public VROInputControllerBase {

public:
    VROInputControllerWasm(std::shared_ptr<VRODriver> driver) : VROInputControllerBase(driver) {}
    virtual ~VROInputControllerWasm() {}
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
        return std::make_shared<VROInputPresenterWasm>();
    }

private:
    
    void updateOrientation(const VROCamera &camera);
    
};
#endif
