//
//  VROTestUtil.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROTestUtil_h
#define VROTestUtil_h

#include <stdio.h>
#include <memory>
#include <string>
#include <functional>
#include "VROTexture.h"

class VROTexture;
class VROVector3f;
class VRONode;
class VROVideoTexture;

class VROTestUtil {
public:
    
    static std::string getURLForResource(std::string resource, std::string type);
    static void *loadDataForResource(std::string resource, std::string type, int *outLength);

    static std::shared_ptr<VROTexture> loadCloudBackground();
    static std::shared_ptr<VROTexture> loadNiagaraBackground();
    static std::shared_ptr<VROTexture> loadWestlakeBackground();
    
    static std::shared_ptr<VROTexture> loadDiffuseTexture(std::string texture,
                                                          VROMipmapMode mipmap = VROMipmapMode::Runtime,
                                                          VROStereoMode stereo = VROStereoMode::None);
    static std::shared_ptr<VROTexture> loadSpecularTexture(std::string texture);
    static std::shared_ptr<VROTexture> loadNormalTexture(std::string texture);
    static std::shared_ptr<VROTexture> loadTexture(std::string texture, bool sRGB);
    
    static std::shared_ptr<VROTexture> loadRadianceHDRTexture(std::string texture);
    static std::shared_ptr<VROTexture> loadHDRTexture(std::string texture);
    
    static std::shared_ptr<VRONode> loadFBXModel(std::string model, VROVector3f position, VROVector3f scale,
                                                 int lightMask, std::string animation);
    static void setLightMasks(std::shared_ptr<VRONode> node, int value);
    
    static std::shared_ptr<VROVideoTexture> loadVideoTexture(std::shared_ptr<VRODriver> driver,
                                                             std::function<void(std::shared_ptr<VROVideoTexture>)> callback,
                                                             VROStereoMode stereo = VROStereoMode::None);

private:
    
    static void animateTake(std::weak_ptr<VRONode> node_w, std::string name);
    
};

#endif /* VROTestUtil_h */
