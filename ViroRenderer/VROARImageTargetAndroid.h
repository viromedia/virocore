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
    /*
     Unlike the iOS version of this class we have a string ID here because in the imperative case
     we need to be able to return an anchor and allow the user to match their target w/ the anchor.
     iOS doesn't need this because it's solely a declarative API for image markers.
     */
    VROARImageTargetAndroid(jobject bitmapImage, VROImageOrientation orientation,
                            float physicalWidth, std::string id);

    VROARImageTargetAndroid(std::string id);

    virtual ~VROARImageTargetAndroid();

    void initWithTrackingImpl(VROImageTrackingImpl impl);

    std::string getId() const {
        return _id;
    }

    /*
     This will return nullptr if the target was found from the image database
     */
    std::shared_ptr<VROImage> getImage() {
        return _image;
    }

private:
    std::shared_ptr<VROImage> _image;
    std::string _id;

    VROImageTrackingImpl _currentImpl;

};


#endif //ANDROID_VROARIMAGETARGETANDROID_H
