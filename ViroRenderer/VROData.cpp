//
//  VROData.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROData.h"

VROData::VROData(void *data, int dataLength, bool copy) {
    if (copy) {
        _data = malloc(dataLength);
        memcpy(_data, data, dataLength);
    }
    else {
        _data = data;
    }
}

VROData::~VROData() {
    free (_data);
}