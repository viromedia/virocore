//
//  VROFBXLoader.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROFBXLoader.h"
#include "VRONode.h"
#include "VROPlatformUtil.h"
#include "VROGeometry.h"

std::shared_ptr<VRONode> VROFBXLoader::loadFBXFromURL(std::string url, std::string baseURL,
                                                      bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([url, baseURL, node, onFinish] {
            bool isTemp = false;
            bool success = false;
            std::string file = VROPlatformDownloadURLToFile(url, &isTemp, &success);
            
            std::shared_ptr<VROGeometry> geometry;
            if (success) {
                geometry = loadFBX(file, baseURL, true, nullptr);
            }
            if (isTemp) {
                VROPlatformDeleteFile(file);
            }
            
            VROPlatformDispatchAsyncRenderer([node, geometry, onFinish] {
                injectFBX(geometry, node, onFinish);
            });
        });
    }
    else {
        bool isTemp = false;
        bool success = false;
        std::string file = VROPlatformDownloadURLToFile(url, &isTemp, &success);
        
        std::shared_ptr<VROGeometry> geometry;
        if (success) {
            geometry = loadFBX(file, baseURL, true, nullptr);
        }
        if (isTemp) {
            VROPlatformDeleteFile(file);
        }
        
        injectFBX(geometry, node, onFinish);
    }
    
    return node;
}

std::shared_ptr<VRONode> VROFBXLoader::loadFBXFromFile(std::string file, std::string baseDir,
                                                       bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([file, baseDir, node, onFinish] {
            std::shared_ptr<VROGeometry> geometry = loadFBX(file, baseDir, false, nullptr);
            VROPlatformDispatchAsyncRenderer([node, geometry, onFinish] {
                injectFBX(geometry, node, onFinish);
            });
        });
    }
    else {
        std::shared_ptr<VROGeometry> geometry = loadFBX(file, baseDir, false, nullptr);
        injectFBX(geometry, node, onFinish);
    }
    
    return node;
}

std::shared_ptr<VRONode> VROFBXLoader::loadFBXFromFileWithResources(std::string file, std::map<std::string, std::string> resourceMap,
                                                                    bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([file, resourceMap, node, onFinish] {
            std::shared_ptr<VROGeometry> geometry = loadFBX(file, "", false, &resourceMap);
            VROPlatformDispatchAsyncRenderer([node, geometry, onFinish] {
                injectFBX(geometry, node, onFinish);
            });
        });
    }
    else {
        std::shared_ptr<VROGeometry> geometry = loadFBX(file, "", false, &resourceMap);
        injectFBX(geometry, node, onFinish);
    }
    
    return node;
}

void VROFBXLoader::injectFBX(std::shared_ptr<VROGeometry> geometry, std::shared_ptr<VRONode> node,
                             std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
    
    // TODO inject subnodes as well
    
    if (geometry) {
        node->setGeometry(geometry);
        if (onFinish) {
            onFinish(node, true);
        }
    }
    else {
        if (onFinish) {
            onFinish(node, false);
        }
    }
}

std::shared_ptr<VROGeometry> VROFBXLoader::loadFBX(std::string file, std::string base, bool isBaseURL,
                                                   const std::map<std::string, std::string> *resourceMap) {
    
    pinfo("Loading FBX from file %s", file.c_str());
    
    
    std::string data = VROPlatformLoadFileAsString(file);
    
    viro::Node node;
    if (!node.ParseFromString(data)) {
        pinfo("Failed to parse FBX protobuf");
        return {};
    }
    
    pinfo("Read FBX protobuf");
    pinfo("Num vertices %d", node.geometry().source(0).vertex_count());
    
    return {};
}
