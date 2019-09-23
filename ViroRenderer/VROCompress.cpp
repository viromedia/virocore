//
//  VROCompress.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/6/17.
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
        pwarn("inflateInit failed while decompressing");
        return "";
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
        pwarn("Error during zlib decompression [ret: %d, message: %s]", ret, zs.msg);
        return "";
    }
    
    return outstring;
}
