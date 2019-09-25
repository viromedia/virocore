//
//  VROApiKeyMetrics.m
//  ViroRenderer
//
//  Created by Andy Chu on 10/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

- (id)initWithApiKey:(NSString *)apiKey
            platform:(NSString *)platform {
    self = [super init];
    if (self) {
        NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
        BOOL isDebug = [[[[NSBundle mainBundle] appStoreReceiptURL] lastPathComponent] isEqualToString:@"sandboxReceipt"];
        // Override the above value if the bundle id is our own testbed app
        if ([bundleId caseInsensitiveCompare:@"com.viromedia.ViroMedia"] == NSOrderedSame) {
          isDebug = false;
        }
        NSString *buildType = isDebug ? @"debug" : @"release";

      // we care more about the actual headset than the platform
        _ApiKey_BundleId_BuildType = [NSString stringWithFormat:kVROApiKeyMetricsFormat, apiKey, @"ios", platform, bundleId, buildType];
        NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"yyyyMMdd"];
        _Date = [dateFormatter stringFromDate:[NSDate date]];
        _Count = 0;
    }
    return self;
}

@end
