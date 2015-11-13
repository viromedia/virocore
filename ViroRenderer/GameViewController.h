//
//  GameViewController.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "VROViewController.h"
#import "VROCrossLayout.h"

@interface GameViewController : NSObject <VROStereoRendererDelegate>

@end

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


