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
        _screen(screen),
        _device() {
        
    }
    
    ~VROHeadMountedDisplay() {

    }
    
    const VROScreen &getScreen() const {
        return _screen;
    }
    const VRODevice &getDevice() const {
        return _device;
    }
    
    bool equals(const VROHeadMountedDisplay *other) const {
        if (other == nullptr) {
            return false;
        }
        else if (other == this) {
            return true;
        }
        
        return _screen.equals(&other->_screen) &&
               _device.equals(&other->_device);
    }
    
private:
    
    VROScreen _screen;
    VRODevice _device;
    
};

#endif /* HeadMountedDisplay_h */
