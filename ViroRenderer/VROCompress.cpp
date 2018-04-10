//
//  VROCompress.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROCompress.h"
#include "VROLog.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

std::string VROCompress::compress(const std::string &str, int compressionlevel) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (deflateInit(&zs, compressionlevel) != Z_OK) {
        pabort("deflateInit failed while compressing.");
    }
    
    zs.next_in = (Bytef*)str.data();
    zs.avail_in = (unsigned int)str.size();
    
    int ret;
    char outbuffer[32768];
    std::string outstring;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        
        ret = deflate(&zs, Z_FINISH);
        
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer,
                             zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);
    
    deflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        pabort();
    }
    
    return outstring;
}

std::string VROCompress::decompress(const std::string &str) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (inflateInit(&zs) != Z_OK) {
        pabort("inflateInit failed while decompressing.");
    }
    
    zs.next_in = (Bytef*)str.data();
    zs.avail_in = (unsigned int)str.size();
    
    int ret;
    char outbuffer[32768];
    std::string outstring;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        
        ret = inflate(&zs, 0);
        
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer,
                             zs.total_out - outstring.size());
        }
        
    } while (ret == Z_OK);
    
    inflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        pinfo("Error during zlib decompression [ret: %d, message: %s]", ret, zs.msg);
        pabort("Error during zlib decompression, see above for error log");
    }
    
    return outstring;
}
