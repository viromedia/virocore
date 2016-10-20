//
//  VROApiKey.m
//  ViroRenderer
//
//  Created by Andy Chu on 10/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROApiKey.h"

@implementation VROApiKey

+ (NSString *)dynamoDBTableName {
  return @"ApiKeys_Alpha";
}

+ (NSString *)hashKeyAttribute {
  return @"ApiKey";
}


@end
