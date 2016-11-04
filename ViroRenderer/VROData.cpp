//
//  VROData.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROData.h"

VROData::VROData(void *data, int dataLength, VRODataOwnership ownership) :
    _ownership(ownership) {
        
    if (ownership == VRODataOwnership::Copy) {
        _data = malloc(dataLength);
        _dataLength = dataLength;
        memcpy(_data, data, dataLength);
    }
    else {
        _data = data;
        _dataLength = dataLength;
    }
}

VROData::VROData(const void *data, int dataLength) :
    _ownership(VRODataOwnership::Copy) {
    _data = malloc(dataLength);
    _dataLength = dataLength;
    memcpy(_data, data, dataLength);
}

VROData::~VROData() {
    if (_ownership == VRODataOwnership::Copy || _ownership == VRODataOwnership::Move) {
        free (_data);
    }
}