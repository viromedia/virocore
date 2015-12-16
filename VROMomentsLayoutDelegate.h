//
//  VROMomentsLayoutDelegate.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/16/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMomentsLayoutDelegate_h
#define VROMomentsLayoutDelegate_h

#import <ViroKit/ViroKit.h>

class VROMomentsLayoutDelegate : public VROCrossLayoutDelegate {
    
public:
    
    VROMomentsLayoutDelegate(std::shared_ptr<VROLayer> center,
                             std::shared_ptr<VROLayer> top,
                             std::shared_ptr<VROLayer> bottom,
                             std::shared_ptr<VROLayer> left,
                             std::shared_ptr<VROLayer> right) :
    centerLayer(center),
    topLayer(top),
    bottomLayer(bottom),
    leftLayer(left),
    rightLayer(right)
    {}
    
    virtual std::shared_ptr<VROLayer> getCenterLayer();
    virtual std::shared_ptr<VROLayer> getTopLayer()   ;
    virtual std::shared_ptr<VROLayer> getBottomLayer();
    virtual std::shared_ptr<VROLayer> getLeftLayer()  ;
    virtual std::shared_ptr<VROLayer> getRightLayer() ;
    
private:
    
    std::shared_ptr<VROLayer> centerLayer;
    std::shared_ptr<VROLayer> topLayer;
    std::shared_ptr<VROLayer> bottomLayer;
    std::shared_ptr<VROLayer> leftLayer;
    std::shared_ptr<VROLayer> rightLayer;
    
};

#endif /* VROMomentsLayoutDelegate_h */
