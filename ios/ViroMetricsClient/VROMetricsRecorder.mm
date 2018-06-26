//
//  VROMetricsRecorder.m
//  ViroRenderer
//
//  Created by Manish Bodhankar on 4/11/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#import "VROMetricsRecorder.h"

static VROMetricsRecorder *sharedClient;
NSString * const kApplicationUUIDKey = @"com.viromedia.app.uuid.key";

@implementation VROMetricsRecorder
+ (VROMetricsRecorder *)sharedClientWithViewType:(NSString *)viewType
                                     platform:(NSString *)platform {
  if (!sharedClient) {
    sharedClient = [[VROMetricsRecorder alloc] initWithViewType:viewType platform:platform];
  }
  return sharedClient;
}
- (id)initWithViewType:(NSString *)viewType
              platform:(NSString *)platform {
  self = [super init];
  if (self) {
    self.viewType = [[NSString alloc] initWithString:viewType];
    self.platform = [[NSString alloc] initWithString:platform];
    
    [KeenClient sharedClientWithProjectID:@"5ab1966bc9e77c0001b45ba0" andWriteKey:@"715EDB702D9AD29A56E127F08864DBCED277F35D946AA4DD2D4BD712B2356CA9E2E17276E74A8B69E44A720D0F92AF3A0D81CAF61404ABA069EE794C3FBFC19F5D66CB32B192B0B43AEA6CA61CED029E564237DB7452DDDC6955103CFAC11320" andReadKey: nil];
  }
  return self;
}

- (void)recordEvent:(NSString *)event {
  NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
  BOOL isLive = [self isRunningLive];
  // Override the above value if the bundle id is our own testbed app
  if ([bundleId caseInsensitiveCompare:@"com.viromedia.ViroMedia"] == NSOrderedSame) {
    isLive = true;
  }
  NSString *buildType = isLive ? @"release" : @"debug";
  NSString *instanceID = [self getInstanceId];
  NSDictionary *eventDict = [NSDictionary dictionaryWithObjectsAndKeys:
                         event, @"event",
                         bundleId, @"app_id",
                         @"ios", @"os",
                         instanceID, @"instance_id",
                         buildType, @"build_type",
                         self.viewType, @"view_type",
                         @"VIRO_REACT", @"viro_product",
                         self.platform, @"platform",
                         nil];
  [[KeenClient sharedClient] addEvent:eventDict toEventCollection:@"ViroViewInit" error:nil];
  [[KeenClient sharedClient] uploadWithFinishedBlock:nil];
}

- (BOOL) isRunningLive {
#if TARGET_OS_SIMULATOR
  return NO;
#else
  BOOL isRunningTestFlightBeta = [[[[NSBundle mainBundle] appStoreReceiptURL] lastPathComponent] isEqualToString:@"sandboxReceipt"];
  BOOL hasEmbeddedMobileProvision = !![[NSBundle mainBundle] pathForResource:@"embedded" ofType:@"mobileprovision"];
  if (isRunningTestFlightBeta || hasEmbeddedMobileProvision)
    return NO;
  return YES;
#endif
}

- (NSString *)getInstanceId {
  NSString *UUID = [[NSUserDefaults standardUserDefaults] objectForKey:kApplicationUUIDKey];
  if (!UUID) {
    NSString *UUID = [[NSUUID UUID] UUIDString];
    [[NSUserDefaults standardUserDefaults] setObject:UUID forKey:kApplicationUUIDKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
  }
  return UUID;
}
@end
