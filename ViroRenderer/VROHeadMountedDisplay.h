//
//  VROHeadMountedDisplay.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef HeadMountedDisplay_h
#define HeadMountedDisplay_h

#include "VRODevice.h"
#include "VROScreen.h"
#include <memory>

class VROHeadMountedDisplay {
    
public:
    
    VROHeadMountedDisplay(UIScreen *screen) :
        _screen(std::make_shared<VROScreen>(screen)),
        _device(std::make_shared<VRODevice>()){
        
    }
    
    VROHeadMountedDisplay(const VROHeadMountedDisplay *hmd) :
        _screen(hmd->getScreen()),
        _device(hmd->getDevice()) {
            
    }
    
    ~VROHeadMountedDisplay() {

    }
    
    void setScreen(std::shared_ptr<VROScreen> screen) {
        _screen = screen;
    }
    std::shared_ptr<VROScreen> getScreen() const {
        return _screen;
    }
    
    void setDevice(std::shared_ptr<VRODevice> device) {
        _device = device;
    }
    std::shared_ptr<VRODevice> getDevice() const {
        return _device;
    }
    
    bool equals(const VROHeadMountedDisplay *other) const {
        if (other == nullptr) {
            return false;
        }
        else if (other == this) {
            return true;
        }
        
        return _screen->equals(other->_screen.get()) &&
               _device->equals(other->_device.get());
    }
    
private:
    
    std::shared_ptr<VROScreen> _screen;
    std::shared_ptr<VRODevice> _device;
    
};

#endif /* HeadMountedDisplay_h */
