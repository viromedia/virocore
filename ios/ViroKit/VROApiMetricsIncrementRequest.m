//
//  VROApiMetricsIncrementRequest.m
//  ViroRenderer
//
//  Created by Andy Chu on 10/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROApiMetricsIncrementRequest.h"

static NSInteger const kVRORequestMinRetryDelay = 2; // seconds
static NSInteger const kVRORequestMaxRetryDelay = 64; // seconds
static NSInteger const kVRORequestMaxRetryCount = 10;

typedef NS_ENUM(NSUInteger, VROApiMetricsRequestType) {
  // checks to see if the item already exists in the metrics table
  VROApiMetricsRequestTypeCheckForItem,
  
  // adds a new item to the metrics table
  VROApiMetricsRequestTypeAdd,
  
  // increments an item in the metrics table
  VROApiMetricsRequestTypeIncrement,
};

@interface VROApiMetricsIncrementRequest ()

@property (nonatomic, readwrite) AWSDynamoDBObjectMapper *dynamoObjectMapper;
@property (nonatomic, strong) VROApiKeyMetrics *metricsRecord;
@property (nonatomic, assign) VROApiMetricsRequestType requestType;
@property (nonatomic, assign) BOOL hasStarted; // whether or not this request has been started.
@property (nonatomic, assign) NSInteger attempt;

@end

@implementation VROApiMetricsIncrementRequest

- (id)initWithMetrics:(VROApiKeyMetrics *)metricsRecord
      dynamoObjMapper:(AWSDynamoDBObjectMapper *)objMapper {
    self = [super init];
    if (self) {
        _dynamoObjectMapper = objMapper;
        _metricsRecord = metricsRecord;
        // the first request should always be a check. Based on that we either add or increment the item.
        _requestType = VROApiMetricsRequestTypeCheckForItem;
        _hasStarted = NO;
        _attempt = 0;
    }
    return self;
}

- (void)run {
    if (self.hasStarted) {
        return;
    }

    [self runInternal];
}

- (void)runInternal {
    self.attempt++;
    // if we tried more than the max times to post the metrics, just give up. To get here the user
    // has a valid key, so let's not take up more cycles.
    if (self.attempt > kVRORequestMaxRetryCount) {
        return;
    }
  
    // calculate the delay, 0 seconds if it's the first time, otherwise MIN(maxDelay, minDelay^attempt-1)
    NSInteger delay = self.attempt == 1 ? 0 : MIN(kVRORequestMaxRetryDelay, pow(kVRORequestMinRetryDelay, self.attempt - 1));

    NSLog(@"[ApiKeyMetrics] Attempt %ld, performing request type %lu in %ld seconds",
          (long)self.attempt, (unsigned long)self.requestType, (long)delay);
    dispatch_time_t delayTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC));
    dispatch_after(delayTime, dispatch_get_main_queue(), ^(void) {
        if (self.requestType == VROApiMetricsRequestTypeCheckForItem) {
          [self checkForItem];
        } else if (self.requestType == VROApiMetricsRequestTypeAdd) {
          [self addMetric];
        } else if (self.requestType == VROApiMetricsRequestTypeIncrement) {
          [self incrementMetric];
        }
    });
}

- (void)checkForItem {

    AWSTask *checkForItemTask = [self.dynamoObjectMapper load:[VROApiKeyMetrics class]
                                                      hashKey:self.metricsRecord.ApiKey_BundleId_BuildType
                                                     rangeKey:self.metricsRecord.Date];
    [checkForItemTask continueWithBlock:^id(AWSTask *task) {
        // if the check for an item was successful, then prepare for either an add or increment request.
        if (!task.cancelled && !task.faulted) {
            self.attempt = 0;
            if (task.result) {
                // if we got a result back, then setup for an increment request
                self.requestType = VROApiMetricsRequestTypeIncrement;
            } else {
                // If we didn't get a record back, then prepare for an add request
                self.requestType = VROApiMetricsRequestTypeAdd;
            }
        }
        // regardless of what happened, we need to continue to run (either a retry or a new request)
        [self runInternal];
        return nil;
    }];
}

/**
 This function attempts to add a new item to the DynamoDB table, if it fails to add the item because someone
 else has added the item, then it sets the requestType to increment before restarting.
 */
- (void)addMetric {

    // Set up and create the various values we need for the update input
    AWSDynamoDBAttributeValue *hashKeyValue = [AWSDynamoDBAttributeValue new];
    hashKeyValue.S = self.metricsRecord.ApiKey_BundleId_BuildType;
    
    AWSDynamoDBAttributeValue *rangeKeyValue = [AWSDynamoDBAttributeValue new];
    rangeKeyValue.S = self.metricsRecord.Date;
    
    AWSDynamoDBExpectedAttributeValue *expectNotExists = [AWSDynamoDBExpectedAttributeValue new];
    expectNotExists.exists = [NSNumber numberWithBool:NO];
    
    AWSDynamoDBAttributeValue *newCount = [AWSDynamoDBAttributeValue new];
    newCount.N = [[NSNumber numberWithInteger:1] stringValue];
    
    AWSDynamoDBAttributeValueUpdate *countUpdate = [AWSDynamoDBAttributeValueUpdate new];
    countUpdate.value = newCount;
    countUpdate.action = AWSDynamoDBAttributeActionPut;
    
    // Create the update input
    AWSDynamoDBUpdateItemInput *updateInput = [AWSDynamoDBUpdateItemInput new];
    updateInput.tableName = [VROApiKeyMetrics dynamoDBTableName];
    updateInput.key = @{[VROApiKeyMetrics hashKeyAttribute] : hashKeyValue,
                        [VROApiKeyMetrics rangeKeyAttribute] : rangeKeyValue};
    updateInput.attributeUpdates = @{[VROApiKeyMetrics CountAttribute] : countUpdate};
    updateInput.expected = @{[VROApiKeyMetrics CountAttribute] : expectNotExists};
    updateInput.returnValues = AWSDynamoDBReturnValueNone;
    
    // Execute the update
    AWSDynamoDB *dynamoDB = [AWSDynamoDB defaultDynamoDB];
    [[dynamoDB updateItem:updateInput] continueWithBlock:^id(AWSTask *task) {
        // Retry if the task failed or was cancelled.
        if (task.isFaulted || task.isCancelled) {
            // the task failed its condition which was that the row is missing, so perform an
            // increment on the newly added row instead.
            if (task.error && task.error.code == AWSDynamoDBErrorConditionalCheckFailed) {
                self.requestType = VROApiMetricsRequestTypeIncrement;
            }
            [self runInternal];
        }
        return nil;
    }];
}

/**
 This function attempts to increment the item in the metric table that corresponds to self.metricsRecord.
 */
- (void)incrementMetric {

    // Setup and create the values for the update input
    AWSDynamoDBAttributeValue *hashKeyValue = [AWSDynamoDBAttributeValue new];
    hashKeyValue.S = self.metricsRecord.ApiKey_BundleId_BuildType;
    
    AWSDynamoDBAttributeValue *rangeKeyValue = [AWSDynamoDBAttributeValue new];
    rangeKeyValue.S = self.metricsRecord.Date;
    
    AWSDynamoDBAttributeValue *incrementValue = [AWSDynamoDBAttributeValue new];
    incrementValue.N = @"1";
    
    // Assemble the updateInput
    AWSDynamoDBUpdateItemInput *updateInput = [AWSDynamoDBUpdateItemInput new];
    updateInput.tableName = [VROApiKeyMetrics dynamoDBTableName];
    updateInput.key = @{[VROApiKeyMetrics hashKeyAttribute] : hashKeyValue,
                        [VROApiKeyMetrics rangeKeyAttribute] : rangeKeyValue};
    
    // This expression is saying to add the :val to the #count, where :val is defined in
    // expressionAttributeValues and #count is defined in expressionAttributeNames.
    // You can usually avoid using #count, but in this case 'count' is a dynamo keyword
    updateInput.updateExpression = @"ADD #count :val";
    updateInput.expressionAttributeNames = @{@"#count" : [VROApiKeyMetrics CountAttribute]};
    updateInput.expressionAttributeValues = @{@":val" : incrementValue};
    
    // Execute the update
    AWSDynamoDB *dynamoDB = [AWSDynamoDB defaultDynamoDB];
    [[dynamoDB updateItem:updateInput] continueWithBlock:^id(AWSTask *task) {
        // Retry if the task failed or was cancelled.
        if (task.isFaulted || task.isCancelled) {
            [self runInternal];
        }
        return nil;
    }];
}

@end
