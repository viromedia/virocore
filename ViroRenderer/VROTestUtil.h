//
//  VROTestUtil.h
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
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
    
    static std::shared_ptr<VRONode> loadFBXModel(std::string model, VROVector3f position, VROVector3f scale, VROVector3f rotation,
                                                 int lightMask, std::string animation, std::shared_ptr<VRODriver> driver,
                                                 std::function<void(std::shared_ptr<VRONode>, bool)> onFinish);
    static std::shared_ptr<VRONode> loadGLTFModel(std::string model, std::string ext, VROVector3f position, VROVector3f scale,
                                                       int lightMask, std::string animation, std::shared_ptr<VRODriver> driver,
                                                  std::function<void(std::shared_ptr<VRONode>, bool)> onFinish);
    static void setLightMasks(std::shared_ptr<VRONode> node, int value);
    
    static std::shared_ptr<VROVideoTexture> loadVideoTexture(std::shared_ptr<VRODriver> driver,
                                                             std::function<void(std::shared_ptr<VROVideoTexture>)> callback,
                                                             VROStereoMode stereo = VROStereoMode::None);

private:
    
    static void animateTake(std::weak_ptr<VRONode> node_w, std::string name);
    
};

#endif /* VROTestUtil_h */
