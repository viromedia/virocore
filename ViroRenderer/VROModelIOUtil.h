//
//  VROModelIOUtil.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROModelIOUtil_h
#define VROModelIOUtil_h

#include <string>
#include <map>
#include <memory>

class VROTexture;

/*
 Static utilities for Model IO.
 */
class VROModelIOUtil {
    
public:

    /*
     Load the texture with the given name, from the given base path (or base URL). First check
     if the texture exists in the provided resourceMap or cache. If the texture could not be
     loaded, returns an empty shared_ptr.
     
     Set sRGB to true to gamma-uncorrect the texture into linear RGB when sampling. This should
     only be used for color (diffuse) textures, and not for textures that are *already* linear
     (e.g. specular, normal, etc.).
     */
    static std::shared_ptr<VROTexture> loadTexture(const std::string &name, std::string &base, bool isBaseURL, bool sRGB,
                                                   const std::map<std::string, std::string> *resourceMap,
                                                   std::map<std::string, std::shared_ptr<VROTexture>> &cache);
    
};

#endif /* VROModelIOUtil_h */
