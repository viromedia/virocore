//
//  VROOBJLoader.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/13/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROOBJLoader_h
#define VROOBJLoader_h

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"

class VRONode;

class VROOBJLoader {
    
public:
    
    /*
     Load the OBJ file at the given URL or file. For all dependent resources
     (e.g. textures) found, locate them by prepending the given baseURL or baseDir.
     */
    static std::shared_ptr<VRONode> loadOBJFromURL(std::string url, std::string baseURL);
    static std::shared_ptr<VRONode> loadOBJFromFile(std::string file, std::string baseDir);
    
};

#endif /* VROOBJLoader_h */
