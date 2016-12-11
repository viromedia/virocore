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

    // Simple string compare, not unicode safe (since there are multiple ways of representing some
    // characters and this function does a character-by-character comparison.
    static bool strcmpinsensitive(const std::string& a, const std::string& b);

private:
    
};

#endif /* VROStringUtil_h */
