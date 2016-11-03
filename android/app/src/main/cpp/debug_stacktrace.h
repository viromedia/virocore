/*
 * debug_stacktrace.h
 *
 * Copyright (c) 2013 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * Use is subject to license terms.
 */
#pragma once

class DebugStacktrace {
public:
    enum {
        MAX_DEPTH = 31
    };

    DebugStacktrace(const DebugStacktrace&) = delete;
    DebugStacktrace& operator=(const DebugStacktrace&) = delete;

    void logStacktrace(unsigned int ignoreFrames);

    static DebugStacktrace& getInstance();

    void* getPimpl() {
        return mPimpl;
    }
private:
    DebugStacktrace();
    ~DebugStacktrace();
    void* mPimpl;

};

