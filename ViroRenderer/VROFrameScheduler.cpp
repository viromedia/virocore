//
//  VROFrameScheduler.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROFrameScheduler.h"
#include "VROLog.h"

VROFrameScheduler::VROFrameScheduler() {
    
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
        }
    }
}
