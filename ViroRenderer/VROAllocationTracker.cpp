//
//  VROAllocationTracker.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#include "VROAllocationTracker.h"
#include "VROTime.h"
#include "VROLog.h"
#include <mutex>
#include <map>
#include <limits>
#include <atomic>
#include "VROTaskQueue.h"

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
    pinfo("    Video Tex Caches:    %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::VideoTextureCaches)].load()));
    pinfo("    Typefaces:           %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Typefaces)].load()));
    pinfo("    Glyphs:              %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Glyphs)].load()));
    pinfo("    Glyph Atlases:       %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::GlyphAtlases)].load()));
    pinfo("    Render Targets:      %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::RenderTargets)].load()));
    pinfo("    VBO:                 %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::VBO)].load()));
    pinfo("    Task Queues:         %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::TaskQueues)].load()));
    pinfo("    Anchors:             %d", (sBytesAllocated[static_cast<int>(VROAllocationBucket::Anchors)].load()));
    VROTaskQueue::printTaskQueues();
}
