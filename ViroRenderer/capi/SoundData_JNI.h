//
//  SoundData_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef SoundData_JNI_h
#define SoundData_JNI_h

#include <memory>
#include <VROSoundDataGVR.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class VROSoundDataDelegate_JNI: public VROSoundDataDelegate {
public:
    VROSoundDataDelegate_JNI(VRO_OBJECT soundDataObject, VRO_ENV env);
    ~VROSoundDataDelegate_JNI();

    void dataIsReady();
    void dataError(std::string error);
private:
    VRO_OBJECT _javaObject;
};

#endif