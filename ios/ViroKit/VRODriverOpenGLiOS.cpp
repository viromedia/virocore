//
//  VRODriverOpenGLiOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 12/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VRODriverOpenGLiOS.h"
#include "vr/gvr/capi/include/gvr_audio.h"

void VRODriverOpenGLiOS::setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                                      std::string ceilingMaterial, std::string floorMaterial) {
    if (sizeX == 0 && sizeY == 0 && sizeZ == 0) {
        _gvrAudio->EnableRoom(false);
    } else {
        _gvrAudio->EnableRoom(true);
        _gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
                                     (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(wallMaterial),
                                     (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(ceilingMaterial),
                                     (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(floorMaterial));
    }
}
