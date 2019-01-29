//
//  VROAnimBodyDataiOS.cpp
//  ViroRenderer
//
//  Created by vik.advani on 1/25/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBodyPlayer.h"
#include "VROAnimBodyDataiOS.h"
#include "VROTime.h"


VROBodyAnimDataRecorderiOS::VROBodyAnimDataRecorderiOS() {
    _nsBodyAnimTotalTime = [NSString stringWithUTF8String:kBodyAnimTotalTime.c_str()];
    _nsBodyAnimAnimRows = [NSString stringWithUTF8String:kBodyAnimAnimRows.c_str()];
    _nsBodyAnimTimestamp = [NSString stringWithUTF8String:kBodyAnimTimestamp.c_str()];
    _nsBodyAnimJoints = [NSString stringWithUTF8String:kBodyAnimJoints.c_str()];
    _nsBodyVersion = [NSString stringWithUTF8String:kBodyAnimVersion.c_str()];
    _nsBodyAnimInitModelTransform = [NSString stringWithUTF8String:kBodyAnimInitModelTransform.c_str()];
};

void VROBodyAnimDataRecorderiOS::startRecording(VROMatrix4f startWorldTransform) {
    _isRecording = true;
    _startRecordingTime = VROTimeCurrentMillis();
    _initRecordWorldTransformOfRootNode = startWorldTransform;
    _recordedAnimationData = [[NSMutableDictionary alloc] init];
    _recordedAnimationRows = [[NSMutableArray alloc] init];
}

void VROBodyAnimDataRecorderiOS::stopRecording()  {
    _isRecording = false;
    _endRecordingTime = VROTimeCurrentMillis();
  
    //record the version body anim version #.
     [_recordedAnimationData setObject:@"ver_1.0" forKey:_nsBodyVersion];
    
    //record the total time of the animation recorded.
    [_recordedAnimationData setObject:[NSNumber numberWithDouble:_endRecordingTime - _startRecordingTime] forKey:_nsBodyAnimTotalTime];
    
    //record the body joint data.
    [_recordedAnimationData setObject:_recordedAnimationRows forKey:_nsBodyAnimAnimRows];
    
    //record start the world matrix.
    const float *matrixArray = _initRecordWorldTransformOfRootNode.getArray();
    NSMutableArray *ma = [NSMutableArray arrayWithCapacity:16];
    for (int i=0; i<16; i++) {
        [ma addObject:[NSNumber numberWithFloat:matrixArray[i]]];
    }
    
    [_recordedAnimationData setObject:ma forKey:_nsBodyAnimInitModelTransform];
    
}

void VROBodyAnimDataRecorderiOS::beginRecordedRow() {
    _currentRow = [[NSMutableDictionary alloc] init];
    _jointValues = [[NSMutableDictionary alloc] init];
    _animRowDataValues = [[NSMutableDictionary alloc] init];
    
    // write out current timestamp for new row.
    double currentTime = VROTimeCurrentMillis() - _startRecordingTime;
    NSNumber *number = [NSNumber numberWithDouble:currentTime];
    [_animRowDataValues setObject:number forKey:_nsBodyAnimTimestamp];
}

void VROBodyAnimDataRecorderiOS::addJointToRow(std::string jointName, VROVector3f jointPos) {
    NSString *boneNameNS = [NSString stringWithCString:jointName.c_str()
                                              encoding:[NSString defaultCStringEncoding]];

    NSArray *pointsArray = [NSArray arrayWithObjects: [NSNumber numberWithFloat:jointPos.x],[NSNumber numberWithFloat:jointPos.y],[NSNumber numberWithFloat:jointPos.z], nil];
    // write out on the joint values.
    [_jointValues setObject:pointsArray forKey:boneNameNS];
}

void VROBodyAnimDataRecorderiOS::endRecordedRow() {
    [_animRowDataValues setObject:_jointValues forKey:_nsBodyAnimJoints];
    [_recordedAnimationRows addObject:_animRowDataValues];
}

std::string VROBodyAnimDataRecorderiOS::toJSON()  {
    std::string returnJson;
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:_recordedAnimationData
                                                       options:(NSJSONWritingOptions)     NSJSONWritingPrettyPrinted
                                                         error:&error];

    NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    returnJson = std::string([jsonString UTF8String]);
    return returnJson;
}

VROBodyAnimDataReaderiOS::VROBodyAnimDataReaderiOS () {
    _nsBodyAnimTotalTime = [NSString stringWithUTF8String:kBodyAnimTotalTime.c_str()];
    _nsBodyAnimAnimRows = [NSString stringWithUTF8String:kBodyAnimAnimRows.c_str()];
    _nsBodyAnimTimestamp = [NSString stringWithUTF8String:kBodyAnimTimestamp.c_str()];
    _nsBodyAnimJoints = [NSString stringWithUTF8String:kBodyAnimJoints.c_str()];
    _nsBodyVersion = [NSString stringWithUTF8String:kBodyAnimVersion.c_str()];
    _nsBodyAnimInitModelTransform = [NSString stringWithUTF8String:kBodyAnimInitModelTransform.c_str()];
}

std::shared_ptr<VROBodyAnimData> VROBodyAnimDataReaderiOS::fromJSON(std::string animJSON) {
    NSError *error = nil;
    // convert std:string to NSString.
    NSString *animJSString = [NSString stringWithCString:animJSON.c_str()
                                                encoding:[NSString defaultCStringEncoding]];
    NSData *data = [animJSString dataUsingEncoding:NSUTF8StringEncoding];
    // convert NSString to dictionary.
    NSDictionary *animDictionary = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:&error];

    //create our animBodyData object.
    std::shared_ptr<VROBodyAnimData> animBodyData =  std::make_shared<VROBodyAnimData>();

    // load the version # of the anim data.
    loadVersion(animDictionary, animBodyData);

    // set the total time.
    loadTotalTime(animDictionary, animBodyData);
    
    // load in the world start matrix.
    loadWorldStartMatrix(animDictionary, animBodyData);
    
    // load in the animation rows.
    loadAnimRows(animDictionary, animBodyData);
    return animBodyData;
}

void VROBodyAnimDataReaderiOS::loadTotalTime(NSDictionary *dictionary, std::shared_ptr<VROBodyAnimData> animBodyData) {
    
    NSNumber *totalTimeNS = (NSNumber *)dictionary[_nsBodyAnimTotalTime];
    double totalAnimTime = [totalTimeNS doubleValue];
    // set total animination time.
    animBodyData->setTotalTime(totalAnimTime);
}

void VROBodyAnimDataReaderiOS::loadVersion(NSDictionary *dictionary, std::shared_ptr<VROBodyAnimData> animBodyData) {
    NSString *versionNS = dictionary[_nsBodyVersion];
    std::string version = std::string([versionNS UTF8String]);
    animBodyData->setVersion(version);
}

void VROBodyAnimDataReaderiOS::loadWorldStartMatrix(NSDictionary *dictionary, std::shared_ptr<VROBodyAnimData> animBodyData) {
    NSArray *matrixArray = dictionary[_nsBodyAnimInitModelTransform];
    float array[16];
    int i=0;
    for (NSNumber *number in matrixArray) {
        array[i] = [number floatValue];
        i++;
    }

    VROMatrix4f modelStartWorldMatrix(array);
    // set the model start world matrix.
    animBodyData->setModelStartWorldMatrix(modelStartWorldMatrix);
}

void VROBodyAnimDataReaderiOS::loadAnimRows(NSDictionary *dictionary, std::shared_ptr<VROBodyAnimData> animBodyData) {
    NSArray *animRows = (NSArray *)dictionary[_nsBodyAnimAnimRows];
    unsigned long totalLength = [animRows count];
    for (long i =0; i < totalLength; i++) {
        NSDictionary *joints = animRows[i][_nsBodyAnimJoints];
        std::map<VROBodyJointType, VROVector3f> jointMap = loadAnimRow(joints, animBodyData);
        NSNumber *timestamp = animRows[i][_nsBodyAnimTimestamp];
        animBodyData->addAnimRow([timestamp doubleValue], jointMap);
    }
}

std::map<VROBodyJointType, VROVector3f> VROBodyAnimDataReaderiOS::loadAnimRow(NSDictionary *joints, std::shared_ptr<VROBodyAnimData> animBodyData)
{
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
