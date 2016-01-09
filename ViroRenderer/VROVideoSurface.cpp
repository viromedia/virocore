//
//  VROVideoTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoSurface.h"
#include "VROVideoTexture.h"
#include "VROMaterial.h"
#include <memory>

std::shared_ptr<VROVideoSurface> VROVideoSurface::createVideoSurface(float width, float height,
                                                                     NSURL *url, VRORenderContext &context) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    VROSurface::buildGeometry(width, height, sources, elements);
    
    std::shared_ptr<VROVideoSurface> surface = std::shared_ptr<VROVideoSurface>(new VROVideoSurface(sources, elements));
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setWritesToDepthBuffer(true);
    material->setReadsFromDepthBuffer(true);
    
    std::shared_ptr<VROVideoTexture> texture = std::make_shared<VROVideoTexture>();
    texture->displayVideo(url, context);
    material->getDiffuse().setContents(texture);
    
    surface->getMaterials().push_back(material);
    return surface;
}

VROVideoSurface::VROVideoSurface(std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                 std::vector<std::shared_ptr<VROGeometryElement>> &elements) :
    VROSurface(sources, elements) {
        
}

VROVideoSurface::~VROVideoSurface() {

}

