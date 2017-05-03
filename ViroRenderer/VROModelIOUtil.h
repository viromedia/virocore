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
     */
    static std::shared_ptr<VROTexture> loadTexture(const std::string &name, std::string &base, bool isBaseURL,
                                                   const std::map<std::string, std::string> *resourceMap,
                                                   std::map<std::string, std::shared_ptr<VROTexture>> &cache);
    
};

#endif /* VROModelIOUtil_h */
