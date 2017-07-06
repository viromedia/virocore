//
//  VROCompress.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROCompress_hpp
#define VROCompress_hpp

#include <stdio.h>
#include <string>
#include <zlib.h>

class VROCompress {
    
public:
    
    /*
     Compress an STL string using zlib with given compression level and return
     the binary data.
     */
    static std::string compress(const std::string &str,
                                int compressionlevel = Z_BEST_COMPRESSION);
    
    /*
     Decompress an STL string using zlib and return the original data. 
     */
    static std::string decompress(const std::string &str);
    
};

#endif /* VROCompress_hpp */
