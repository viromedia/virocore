//
//  VROFrameScheduler.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/17.
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

#include "VROFrameScheduler.h"
#include "VROLog.h"

// Block and process all tasks when we reach this number
// of starvation frames
static const int kStarvationPurgeFrameCount = 60;

VROFrameScheduler::VROFrameScheduler() :
    _starvationFrameCount(0) {
    
}

VROFrameScheduler::~VROFrameScheduler() {
    
}

bool VROFrameScheduler::isTaskQueued(std::string key) {
    std::lock_guard<std::recursive_mutex> lock(_taskQueueMutex);
    return _queuedTasks.find(key) != _queuedTasks.end();
}

void VROFrameScheduler::scheduleTask(std::string key, std::function<void()> task) {
    std::lock_guard<std::recursive_mutex> lock(_taskQueueMutex);
    
    if (_queuedTasks.find(key) != _queuedTasks.end()) {
        // Task is already queued
        return;
    }
    
    _taskQueue.push({ key, task });
    _queuedTasks.insert(key);
}

void VROFrameScheduler::processTasks(const VROFrameTimer &timer) {
    bool processedAnyTask = false;
    
    while (!_taskQueue.empty()) {
        // The iOS simulator is so slow (due to GPU emulation) we don't bother with waiting
#if !TARGET_OS_SIMULATOR
        if (!timer.isTimeRemainingInFrame()) {
            break;
        }
#endif
        VROFrameTask task;
        
        // Lock the mutex while retrieving the task from the queue
        {
            std::lock_guard<std::recursive_mutex> lock(_taskQueueMutex);
            if (!_taskQueue.empty()) {
                task = _taskQueue.front();
                _taskQueue.pop();
            }
        }
        
        // Process the task outside of the lock
        if (task.functor) {
            task.functor();
            processedAnyTask = true;
        }
    }
    
    if (!_taskQueue.empty() && !processedAnyTask) {
        _starvationFrameCount++;
    }
    
    /*
     If we've been unable to process tasks for this number of frames, 
     block and process them all.
     */
    if (_starvationFrameCount >= kStarvationPurgeFrameCount) {
        pinfo("Tasks starved for %d frames: processing all", _starvationFrameCount);
        
        std::lock_guard<std::recursive_mutex> lock(_taskQueueMutex);
        while (!_taskQueue.empty()) {
            VROFrameTask task = _taskQueue.front();
            _taskQueue.pop();
            
            if (task.functor) {
                task.functor();
            }
        }
        _starvationFrameCount = 0;
    }
}
