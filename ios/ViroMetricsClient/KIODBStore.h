//
//  KIODBStore.h
//  KeenClient
//
//  Created by Cory Watson on 3/26/14.
//  Copyright (c) 2014 Keen Labs. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface KIODBStore : NSObject

 /**
  Reset any pending events so they can be resent.
  */
- (void)resetPendingEventsWithProjectID:(NSString *)projectID;

 /**
  Determine if there are any pending events so the caller can decide what to
  do. See resetPendingEvents or purgePendingEvents.
  */
- (BOOL)hasPendingEventsWithProjectID:(NSString *)projectID;

 /**
  Add an event to the store.
  
  @param eventData Your event data.
  @param coll Your event collection.
  */
- (BOOL)addEvent:(NSData *)eventData collection:(NSString *)eventCollection projectID:(NSString *)projectID;

 /**
  Get a dictionary of events keyed by id that are ready to send to Keen. Events
  that are returned have been flagged as pending in the underlying store.
  */
- (NSMutableDictionary *)getEventsWithMaxAttempts:(int)maxAttempts andProjectID:(NSString *)projectID;

 /**
  Get a count of pending events.
  */
- (NSUInteger)getPendingEventCountWithProjectID:(NSString *)projectID;

 /**
  Get a count of total events, pending or not.
  */
- (NSUInteger)getTotalEventCountWithProjectID:(NSString *)projectID;

 /**
  Purge pending events that were returned from a previous call to getEvents.
  */
- (void)purgePendingEventsWithProjectID:(NSString *)projectID;

 /**
  Delete an event from the store
  
  @param eventId The id of the event to delete.
  */
- (void)deleteEvent: (NSNumber *)eventId;

 /**
  Delete all events from the store
  */
- (void)deleteAllEvents;


 /**
  Increment the `attempts` column
  */
- (void)incrementEventUploadAttempts: (NSNumber *)eventId;

 /**
  Convert an NSDate to ISO-8601 using SQLite (thread safe)
 
  @param date A date.
  */
- (id)convertNSDateToISO8601:(NSDate *)date;

 /**
  Delete events starting at an offset. Helps to keep the "queue" bounded.
 
  @param offset The offset to start deleting events from.
  */
- (void)deleteEventsFromOffset:(NSNumber *)offset;

/**
 Add a query to the store.
 
 @param queryData Your query data.
 @param eventCollection Your event collection.
 @param projectID Your project ID.
 */
- (BOOL)addQuery:(NSData *)queryData queryType:(NSString*)queryType collection:(NSString *)eventCollection projectID:(NSString *)projectID;

/**
 Get a dictionary of the found query parameters.
 
 @param queryData Your query data.
 @param eventCollection Your event collection.
 @param projectID Your project ID.
 */
- (NSMutableDictionary *)getQuery:(NSData *)queryData queryType:(NSString *)queryType collection:(NSString *)eventCollection projectID:(NSString *)projectID;

/**
 Increment the query `attempts` column
 
 @param queryID The query database id.
 */
- (BOOL)incrementQueryAttempts:(NSNumber *)queryID;

/**
 Helper method to find or update the `attempts` column of a query.
 
 @param queryData Your query data.
 @param eventCollection Your event collection.
 @param projectID Your project ID.
 */
- (void)findOrUpdateQuery:(NSData *)queryData queryType:(NSString *)queryType collection:(NSString *)eventCollection projectID:(NSString *)projectID;

/**
 Get a count of total queries.
 
 @param projectID Your project ID.
 */
- (NSUInteger)getTotalQueryCountWithProjectID:(NSString *)projectID;

/**
 Determine if the query in the database has the `attempts` column equal or higher than
 the maxAttempts value.
 
 @param queryData Your query data.
 @param eventCollection Your event collection.
 @param projectID Your project ID.
 @param maxAttempts The threshold for trying a query API call.
 @param queryTTL The threshold in seconds for deleting old queries.
 */
- (BOOL)hasQueryWithMaxAttempts:(NSData *)queryData
                      queryType:(NSString *)queryType
                     collection:(NSString *)eventCollection
                      projectID:(NSString *)projectID
                    maxAttempts:(int)maxAttempts
                       queryTTL:(int)queryTTL;

/**
 Delete all the queries older than X seconds.
 
 @param seconds The seconds to delete events older than.
 */
- (void)deleteQueriesOlderThan:(NSNumber *)seconds;

/**
 Delete all queries from the store
 */
- (void)deleteAllQueries;

@end
