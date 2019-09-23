//
//  VRODriverOpenGLiOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 12/5/17.
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

#include "VRODriverOpenGLiOS.h"
#include "vr/gvr/capi/include/gvr_audio.h"

void VRODriverOpenGLiOS::setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                                      std::string ceilingMaterial, std::string floorMaterial) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = activateGVRAudio();

    if (sizeX == 0 && sizeY == 0 && sizeZ == 0) {
        gvrAudio->EnableRoom(false);
    } else {
        gvrAudio->EnableRoom(true);
        gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
                                    (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(wallMaterial),
                                    (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(ceilingMaterial),
                                    (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(floorMaterial));
    }
}
