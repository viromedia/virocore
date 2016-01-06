//
//  VROLoader.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROLoader_h
#define VROLoader_h

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>
#include <ModelIO/ModelIO.h>
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VRORenderContext.h"

class VRONode;
class VROTexture;

class VROLoader {
    
public:
    
    static std::vector<std::shared_ptr<VRONode>> loadURL(std::string path,
                                                         const VRORenderContext &context);
    
private:
    
    static std::shared_ptr<VRONode> loadMesh(MDLMesh *mesh,
                                             const VRORenderContext &context);
    
    static VROGeometrySourceSemantic parseSemantic(NSString *string);
    static std::pair<int, int> parseFormat(MDLVertexFormat format);
    static int parseIndexSize(MDLIndexBitDepth depth);
    static VROGeometryPrimitiveType parsePrimitive(MDLGeometryType type);
    
    static VROVector4f parseColor(CGColorRef colorRef);
    static std::shared_ptr<VROTexture> parseTexture(MDLTextureSampler *sampler);
    
};


#endif /* VROLoader_h */
