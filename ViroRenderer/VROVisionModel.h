//
//  VROVisionModel.h
//  ViroKit
//
//  Created by Raj Advani on 1/16/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROVisionModel_h
#define VROVisionModel_h

class VROARFrame;

/*
 Runs an ML model on each AR frame received.
 */
class VROVisionModel {
public:
    
    /*
     Update the model. This is invoked on each AR frame update.
     */
    virtual void update(const VROARFrame &frame) = 0;
    
};

#endif /* VROVisionModel_h */
