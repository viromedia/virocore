//
//  VROBodyPlayeriOS.h
//  ViroRenderer
//
//  Created by vik.advani on 1/21/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROBodyPlayeriOS_h
#define VROBodyPlayeriOS_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <map>
#include "VROBodyPlayer.h"
#include "VROTime.h"
#include "VROMatrix4f.h"

class VRORenderContext;
class BodyPlaybackInfo {
public:
    BodyPlaybackInfo(NSDictionary *playbackData) {
        _currentBodyAnimPlayback = playbackData;
        _nsBodyAnimTotalTime = [NSString stringWithUTF8String:kBodyAnimTotalTime.c_str()];
        _nsBodyAnimAnimRows = [NSString stringWithUTF8String:kBodyAnimAnimRows.c_str()];
        _nsBodyAnimTimestamp = [NSString stringWithUTF8String:kBodyAnimTimestamp.c_str()];
        _nsBodyAnimJoints = [NSString stringWithUTF8String:kBodyAnimJoints.c_str()];
        _nsBodyAnimInitModelTransform = [NSString stringWithUTF8String:kBodyAnimInitModelTransform.c_str()];
         _matrixArray = _currentBodyAnimPlayback[_nsBodyAnimInitModelTransform];
         NSNumber *totalTime = (NSNumber *)_currentBodyAnimPlayback[_nsBodyAnimTotalTime];
        _animRows = (NSArray *)_currentBodyAnimPlayback[_nsBodyAnimAnimRows];
        _totalPlaybackTime = [totalTime doubleValue];
        _currentPlaybackRow = 0;
        _playStatus = VROBodyPlayerStatus::Initialized;
    }

    std::map<VROBodyJointType, VROVector3f> getCurrentRowJointsAsMap()
    {
        NSDictionary *joints = _animRows[_currentPlaybackRow][_nsBodyAnimJoints];
        int endLoop = static_cast<int>(VROBodyJointType::LeftAnkle) + 1;
        std::map<VROBodyJointType, VROVector3f> jointMap;
        for (int i = static_cast<int>(VROBodyJointType::Top); i < endLoop; i++) {
            VROBodyJointType bodyJointType = static_cast<VROBodyJointType>(i);
            std::string boneName = kVROBodyBoneTags.at(bodyJointType);
            NSString *boneNameNS = [NSString stringWithCString:boneName.c_str()
                                                      encoding:[NSString defaultCStringEncoding]];
            if (joints[boneNameNS] != nil) {
                NSArray *jointArray = joints[boneNameNS];
                NSNumber *num0 = (NSNumber *)jointArray[0];
                NSNumber *num1 = (NSNumber *)jointArray[1];
                NSNumber *num2 = (NSNumber *)jointArray[2];
                VROVector3f jointLocalSpace([num0 floatValue], [num1 floatValue], [num2 floatValue]);
                jointMap[bodyJointType] = jointLocalSpace;
            }
        }
        return jointMap;
    }

    void startPlayback() {
        _currentPlaybackRow = 0;
        _startPlaybackTime = VROTimeCurrentMillis();
        _playStatus = VROBodyPlayerStatus::Start;
    }

    int getCurrentRow() {
        return _currentPlaybackRow;
    }

    double getStartTime() {
        return _startPlaybackTime;
    }

    NSArray *getMatrixStartArray() {
        return _matrixArray;
    }

    double getCurrentRowTimestamp() {
        NSNumber *timestamp = _animRows[_currentPlaybackRow][_nsBodyAnimTimestamp];
        return [timestamp doubleValue];
    }

    void incrementAnimRow() {
        _currentPlaybackRow++;
        long length = [_animRows count];
        if (_currentPlaybackRow > 0 && _currentPlaybackRow < length && _playStatus == VROBodyPlayerStatus::Start) {
            _playStatus = VROBodyPlayerStatus::Playing;
        } else if (_currentPlaybackRow >= length) {
            if ( _playStatus == VROBodyPlayerStatus::Playing) {
                _playStatus = VROBodyPlayerStatus::Finished;
            }
        }
    }

    VROBodyPlayerStatus getPlayStatus() {
        return  _playStatus;
    }

    bool isFinished() {
        long length = [_animRows count];
        if (_currentPlaybackRow >= length) {
            return true;
        }
        return false;
    }

private:
    NSDictionary *_currentBodyAnimPlayback;
    VROMatrix4f _startRootMatrixWorld;
    VROMatrix4f _initPlaybackWorldTransformOfRootNode;
    NSArray *_animRows;
    NSArray *_matrixArray;
    int _currentPlaybackRow;
    VROBodyPlayerStatus _playStatus;
    double _startPlaybackTime;
    double _totalPlaybackTime;
    NSString *_nsBodyAnimTotalTime;
    NSString *_nsBodyAnimAnimRows;
    NSString *_nsBodyAnimTimestamp;
    NSString *_nsBodyAnimJoints;
    NSString *_nsBodyAnimInitModelTransform;
};

class VROBodyPlayeriOS : public VROBodyPlayer {

public:
    VROBodyPlayeriOS();

    virtual ~VROBodyPlayeriOS() {}
    void prepareAnimation(std::string animData);
    void start();
    void stop();
    void setTime();
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:
    BodyPlaybackInfo *_playbackInfo;
};

#endif /* VROBodyPlayeriOS_h */
