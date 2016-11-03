//
//  VROApiKeyMetrics.m
//  ViroRenderer
//
//  Created by Andy Chu on 10/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROApiKeyMetrics.h"

@implementation VROApiKeyMetrics

# pragma mark - AWSDynamoDBModeling protocol implementation

+ (NSString *)dynamoDBTableName {
    return @"ApiKey_Metrics_Alpha";
}

+ (NSString *)hashKeyAttribute {
    return @"ApiKey_BundleId_BuildType";
}

+ (NSString *)rangeKeyAttribute {
    return @"Date";
}

+ (NSString *)CountAttribute {
    return @"Count";
}

# pragma mark - Instance functions

- (id)initWithApiKey:(NSString *)apiKey {
    self = [super init];
    if (self) {
        NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
#if DEBUG
        NSString *buildType = @"debug";
#else
        NSString *buildType = @"release";
#endif
        _ApiKey_BundleId_BuildType = [NSString stringWithFormat:kVROApiKeyMetricsFormat, apiKey, bundleId, buildType];
        NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"yyyyMMdd"];
        _Date = [dateFormatter stringFromDate:[NSDate date]];
        _Count = 0;
    }
    return self;
}

@end
