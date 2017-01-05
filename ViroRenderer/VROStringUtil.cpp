//
//  VROStringUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROStringUtil.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include "VRODefines.h"

std::string VROStringUtil::toString(int i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}

int VROStringUtil::toInt(std::string s) {
    return atoi(s.c_str());
}

float VROStringUtil::toFloat(std::string s) {
    return atof(s.c_str());
}

std::vector<std::string> VROStringUtil::split(const std::string &s,
                                              const std::string &delimiters,
                                              bool emptiesOk) {
    
    std::vector<std::string> result;
    size_t current;
    size_t next = -1;
    
    do {
        if (!emptiesOk) {
            next = s.find_first_not_of(delimiters, next + 1);
            if (next == std::string::npos) {
                break;
            };
            
            next -= 1;
        }
        
        current = next + 1;
        next = s.find_first_of(delimiters, current);
        result.push_back(s.substr(current, next - current));
    }
    while (next != std::string::npos);
    
    return result;
}

bool VROStringUtil::strcmpinsensitive(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (int i = 0; i < a.size(); i++) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }
    return true;
}
