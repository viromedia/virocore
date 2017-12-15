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
    // TODO: VIRO-2464 don't use _gvrAudio because it hasn't been initialized
//    _gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
//                                 (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(wallMaterial),
//                                 (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(ceilingMaterial),
//                                 (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(floorMaterial));
}
