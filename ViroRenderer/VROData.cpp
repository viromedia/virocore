//
//  VROData.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

VROData::VROData(const void *data, int dataLength, int byteOffset) :
    _ownership(VRODataOwnership::Copy) {
    _data = malloc(dataLength);
    _dataLength = dataLength;

    const void *startingDataPoint = ((char*)data) + byteOffset;
    memcpy(_data, startingDataPoint, dataLength);
}

VROData::~VROData() {
    if (_ownership == VRODataOwnership::Copy || _ownership == VRODataOwnership::Move) {
        free (_data);
    }
}
