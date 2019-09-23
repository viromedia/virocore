//
//  VROTaskQueue.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/7/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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

#include "VROTaskQueue.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VRODefines.h"
#include "VROAllocationTracker.h"
#include <mutex>
#include <algorithm>

#if kDebugTaskQueues
static std::mutex sMapMutex;
static std::vector<std::weak_ptr<VROTaskQueue>> _currentTaskQueues;
#endif

void VROTaskQueue::printTaskQueues() {
#if kDebugTaskQueues
    std::lock_guard<std::mutex> guard(sMapMutex);

    pinfo("   Outstanding task queues");
    int index = 0;
    for (std::weak_ptr<VROTaskQueue> queue_w : _currentTaskQueues) {
        std::shared_ptr<VROTaskQueue> queue_s = queue_w.lock();
        if (queue_s) {
            pinfo("   Queue %d: %s", index, queue_s->_name.c_str());
            queue_s->printOutstandingTasks();

            ++index;
        }
    }
#endif
}

VROTaskQueue::VROTaskQueue(std::string name, VROTaskExecutionOrder executionOrder) :
    _started(false),
    _name(name),
    _executionOrder(executionOrder) {
        
    ALLOCATION_TRACKER_ADD(TaskQueues, 1);
}

VROTaskQueue::~VROTaskQueue() {
    ALLOCATION_TRACKER_SUB(TaskQueues, 1);

#if kDebugTaskQueues
    std::lock_guard<std::mutex> guard(sMapMutex);
    auto it = std::find_if(_currentTaskQueues.begin(), _currentTaskQueues.end(), [this](std::weak_ptr<VROTaskQueue> taskQueue_w) {
        std::shared_ptr<VROTaskQueue> q = taskQueue_w.lock();
        if (q && q.get() == this) {
            return true;
        } else {
            return false;
        }
    });
    if (it != _currentTaskQueues.end()) {
        _currentTaskQueues.erase(it);
    }
#endif
}

void VROTaskQueue::addTask(std::function<void()> task) {
    passert (!_started);
    _tasks.push_back(task);
}

void VROTaskQueue::processTasksAsync(std::function<void()> onFinished) {
#if kDebugTaskQueues
    {
        std::lock_guard<std::mutex> guard(sMapMutex);
        std::weak_ptr<VROTaskQueue> p = shared_from_this();
        _currentTaskQueues.push_back(p);
    }
#endif
    
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
        
        // Clear the tasks after we're done: this is important because tasks will often
        // hold a reference back to their parent task queue, so that they can invoke
        // onTaskComplete(). Because of this we have to explicitly clear the tasks out
        // to ensure any strong ref cycles are removed.
        _tasks.clear();
    }
    else if (_executionOrder == VROTaskExecutionOrder::Serial) {
        // Fire off the next task
        _tasks.pop_back();
        std::function<void()> &task = _tasks.back();
        task();
    }
}
