//
//  VROARImageTargetAndroid.h
//  ViroKit
//
//  Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROARIMAGETARGETANDROID_H
#define ANDROID_VROARIMAGETARGETANDROID_H


#include "VROARImageTarget.h"
#include "VROImage.h"

class VROARImageTargetAndroid : public VROARImageTarget {
public:
    VROARImageTargetAndroid(jobject bitmapImage, VROImageOrientation orientation, float physicalWidth);

    virtual ~VROARImageTargetAndroid();

    void initWithTrackingImpl(VROImageTrackingImpl impl);

private:
    std::shared_ptr<VROImage> _image;
    VROImageOrientation _orientation;
    float _physicalWidth;

    VROImageTrackingImpl _currentImpl;
};


#endif //ANDROID_VROARIMAGETARGETANDROID_H
