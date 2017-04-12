//
//  VROAllocationTracker.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROAllocationTracker.h"
#include "VROTime.h"
#include "VROLog.h"
#include <mutex>
#include <map>
#include <limits>
#include <atomic>

#define ALLOCATION_TRACKING_FREQUENCY 1 // in Hz

static std::atomic<uint32_t> sBytesAllocated[static_cast<int>(VROAllocationBucket::NUM_BUCKETS)];

static double sTimeLastLogged = 0.0;
static std::mutex sTimeLastLoggedMutex;

void VROAllocationTracker::set(VROAllocationBucket bucket, uint32_t bytes) {
    sBytesAllocated[static_cast<int>(bucket)].store(bytes);
}

void VROAllocationTracker::add(VROAllocationBucket bucket, uint32_t bytes) {
    sBytesAllocated[static_cast<int>(bucket)] += bytes;
}

void VROAllocationTracker::subtract(VROAllocationBucket bucket, uint32_t bytes) {
    sBytesAllocated[static_cast<int>(bucket)] -= bytes;
}

void VROAllocationTracker::resize(VROAllocationBucket bucket, uint32_t bytesOld, uint32_t bytesNew) {
    subtract(bucket, bytesOld);
    add(bucket, bytesNew);
}

void VROAllocationTracker::print() {
    const double timeNow = VROTimeCurrentMillis();
    {
        std::lock_guard<std::mutex> timeGuard(sTimeLastLoggedMutex);
        if (timeNow - sTimeLastLogged < 1000.0 / ALLOCATION_TRACKING_FREQUENCY) {
            return;
        }
        sTimeLastLogged = timeNow;
    }

    printNow();
}

void VROAllocationTracker::printNow() {
    pinfo("Allocation tracking");
    pinfo("    Scenes:              %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Scenes)].load()));
    pinfo("    Nodes:               %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Nodes)].load()));
    pinfo("    Geometry:            %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Geometry)].load()));
    pinfo("    Materials:           %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Materials)].load()));
    pinfo("    Material Substrates: %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::MaterialSubstrates)].load()));
    pinfo("    Textures:            %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Textures)].load()));
    pinfo("    Texture Substrates:  %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::TextureSubstrates)].load()));
    pinfo("    Shaders:             %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Shaders)].load()));
    pinfo("    Shader Modifiers:    %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::ShaderModifiers)].load()));
    pinfo("    Video Textures:      %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::VideoTextures)].load()));
}
