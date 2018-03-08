//
//  VROTaskQueue.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/7/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTaskQueue.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VRODefines.h"

VROTaskQueue::VROTaskQueue(VROTaskExecutionOrder executionOrder) :
    _started(false),
    _executionOrder(executionOrder) {
}

VROTaskQueue::~VROTaskQueue() {
    
}

void VROTaskQueue::addTask(std::function<void()> task) {
    passert (!_started);
    _tasks.push_back(task);
}

void VROTaskQueue::processTasksAsync(std::function<void()> onFinished) {
    _started = true;
    _onFinished = onFinished;
    _numOpenTasks = (int) _tasks.size();

    // If there are no tasks, immediately call the onFinished handler
    if (_numOpenTasks == 0) {
        onFinished();
    }
    else if (_executionOrder == VROTaskExecutionOrder::Serial) {
        // Perform one task at a time
        std::function<void()> &task = _tasks.back();
        task();
    }
    else {
        // Fire off all tasks at once
        for (auto it = _tasks.begin(); it != _tasks.end(); ++it) {
            std::function<void()> task = *it;
            task();
        }
    }
}

void VROTaskQueue::onTaskComplete() {
    _numOpenTasks--;
    if (_numOpenTasks == 0) {
        _onFinished();
    }
    else if (_executionOrder == VROTaskExecutionOrder::Serial) {
        // Fire off the next task
        _tasks.pop_back();
        std::function<void()> &task = _tasks.back();
        task();
    }
}
