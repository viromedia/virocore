//
//  VROARImageTargetAndroid.h
//  ViroKit
//
//  Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
