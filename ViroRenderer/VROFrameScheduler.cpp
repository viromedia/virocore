//
//  VROFrameScheduler.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

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
        if (!timer.isTimeRemainingInFrame()) {
            break;
        }
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
