//
//  VROStringUtil.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROStringUtil_h
#define VROStringUtil_h

#include <stdio.h>
#include <string>
#include <vector>

class VROStringUtil {
    
public:
    
    static std::string toString(int i);
    
    static std::vector<std::string> split(const std::string &s,
                                          const std::string &delimiters,
                                          bool emptiesOk);
    
private:
    
};

#endif /* VROStringUtil_h */
