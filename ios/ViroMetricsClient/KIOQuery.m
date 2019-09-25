//
//  KIOQuery.m
//  KeenClient
//
//  Created by Heitor Sergent on 4/21/15.
//  Copyright (c) 2015 Keen Labs. All rights reserved.
//

#import "KIOQuery.h"

@implementation KIOQuery

- (id)initWithQuery:(NSString *)queryType andPropertiesDictionary:(NSDictionary *)propertiesDictionary {
    if (![KIOQuery validateQueryType:queryType]) {
        return nil;
    }
    
    self = [self init];
    
    if (self) {
        self.queryType = queryType;
        self.propertiesDictionary = propertiesDictionary;
    }
    
    return self;
}

- (id)initWithQuery:(NSString *)queryType andQueryName:(NSString *)queryName andPropertiesDictionary:(NSDictionary *)propertiesDictionary {
    if (![KIOQuery validateQueryType:queryType]) {
        return nil;
    }
    
    self = [self init];
    
    if (self) {
        self.queryType = queryType;
        self.queryName = queryName;
        self.propertiesDictionary = propertiesDictionary;
    }
    
    return self;
}

+ (BOOL)validateQueryType:(NSString *)queryType {
    // TODO: Validate query type on client side?
    if (!queryType || [queryType length] == 0) {
        return NO;
    }
    return YES;
}

- (NSData *)convertQueryToData {
    NSError *error = nil;
    
    NSData *data = [NSJSONSerialization dataWithJSONObject:self.propertiesDictionary options:0 error:&error];
    
    return data;
}

@end
