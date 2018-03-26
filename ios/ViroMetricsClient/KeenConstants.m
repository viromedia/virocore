//
//  KeenConstants.m
//  KeenClient
//
//  Created by Daniel Kador on 2/12/12.
//  Copyright (c) 2012 Keen Labs. All rights reserved.
//

#import "KeenConstants.h"

NSString * const kKeenServerAddress = @"https://api.keen.io";
NSString * const kKeenApiVersion = @"3.0";

// Keen API constants

NSString * const kKeenNameParam = @"name";
NSString * const kKeenDescriptionParam = @"description";
NSString * const kKeenSuccessParam = @"success";
NSString * const kKeenErrorParam = @"error";
NSString * const kKeenErrorCodeParam = @"error_code";
NSString * const kKeenInvalidCollectionNameError = @"InvalidCollectionNameError";
NSString * const kKeenInvalidPropertyNameError = @"InvalidPropertyNameError";
NSString * const kKeenInvalidPropertyValueError = @"InvalidPropertyValueError";

// Keen constants related to how much data we'll cache on the device before aging it out

// how many events can be stored for a single collection before aging them out
NSUInteger const kKeenMaxEventsPerCollection = 10000;
// how many events to drop when aging out
NSUInteger const kKeenNumberEventsToForget = 100;

// custom domain for NSErrors
NSString * const kKeenErrorDomain = @"io.keen";