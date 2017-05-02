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
    
    static std::shared_ptr<VROTexture> loadTexture(std::string &name, std::string &base, bool isBaseURL,
                                                   const std::map<std::string, std::string> *resourceMap,
                                                   std::map<std::string, std::shared_ptr<VROTexture>> &cache);
    
};

#endif /* VROModelIOUtil_h */
