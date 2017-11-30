//
//  VRODriverOpenGLiOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGLiOS_h
#define VRODriverOpenGLiOS_h

#include "VRODriverOpenGL.h"
#include "VROSoundGVR.h"
#include "VROAudioPlayeriOS.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROTypefaceiOS.h"
#include "VRODisplayOpenGLiOS.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROPlatformUtil.h"

class VRODriverOpenGLiOS : public VRODriverOpenGL {
    
public:
    
    /*
     If the driver is based off a GLKView, pass that in so we can create the
     display render target with it; otherwise, the display render target will
     be derived from the active framebuffer ID.
     */
    VRODriverOpenGLiOS(GLKView *viewGL, EAGLContext *eaglContext, std::shared_ptr<gvr::AudioApi> gvrAudio) :
        _viewGL(viewGL),
        _eaglContext(eaglContext),
        _gvrAudio(gvrAudio) {
    }
    
    virtual ~VRODriverOpenGLiOS() { }
    
    virtual VROColorRenderingMode getColorRenderingMode() {
        return VROColorRenderingMode::Linear;
    }
    
    /*
     On iOS the primary framebuffer (display) is tied to a GLKView. To
     bind the display we have to go through the GLKView.
     */
    virtual std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            GLKView *viewGL = _viewGL;
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGLiOS>(viewGL, driver);
        }
        return _display;
    }
    
    void willRenderFrame(const VRORenderContext &context) {
        // Set the head position into _gvrAudio when spatial sound is supported by iOS
        VRODriverOpenGL::willRenderFrame(context);
    }

    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return std::make_shared<VROVideoTextureCacheOpenGL>(_eaglContext, driver);
    }

    std::shared_ptr<VROSound> newSound(std::string path, VROSoundType type, bool local) {
        std::shared_ptr<VROSound> sound = VROSoundGVR::create(path, _gvrAudio, type, local);
        return sound;
    }

    std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) {
        std::shared_ptr<VROSound> sound = VROSoundGVR::create(data, _gvrAudio, type);
        return sound;
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string path, bool isLocal) {
        // TODO: VIRO-756 make use of local flag (always assumes it's a web file)
        return std::make_shared<VROAudioPlayeriOS>(path, isLocal);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return std::make_shared<VROAudioPlayeriOS>(data);
    }
    
    std::shared_ptr<VROTypeface> newTypeface(std::string typefaceName, int size) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceiOS>(typefaceName, size, driver);
        typeface->loadFace();
        
        return typeface;
    }

    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial) {
        _gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
                                     VROPlatformParseGVRAudioMaterial(wallMaterial),
                                     VROPlatformParseGVRAudioMaterial(ceilingMaterial),
                                     VROPlatformParseGVRAudioMaterial(floorMaterial));
    }
    
protected:
    
    __weak GLKView *_viewGL;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    EAGLContext *_eaglContext;
    
};

#endif /* VRODriverOpenGLiOS_h */
