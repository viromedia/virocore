//
// Created by Andy Chu on 8/20/18.
//

#ifndef ANDROID_ARIMAGEDATABASELOADERDELEGATE_H
#define ANDROID_ARIMAGEDATABASELOADERDELEGATE_H

#include "VRODefines.h"
#include VRO_C_INCLUDE

class ARImageDatabaseLoaderDelegate {
public:
    ARImageDatabaseLoaderDelegate(VRO_OBJECT javaObject, VRO_ENV env);
    ~ARImageDatabaseLoaderDelegate();

    void loadSuccess();
    void loadFailure(std::string error);
private:
    VRO_OBJECT _javaObject;
};

#endif //ANDROID_ARIMAGEDATABASELOADERDELEGATE_H
