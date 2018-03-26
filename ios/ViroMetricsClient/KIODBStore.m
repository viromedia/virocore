//
//  KIOEventStore.m
//  KeenClient
//
//  Created by Cory Watson on 3/26/14.
//  Copyright (c) 2014 Keen Labs. All rights reserved.
//

#import "KeenClient.h"
#import "KIODBStore.h"
#import "KIOEventStore_PrivateMethods.h"
#import "keen_io_sqlite3.h"

@interface KIODBStore()
- (void)closeDB;

// A dispatch queue used for sqlite.
@property (nonatomic) dispatch_queue_t dbQueue;

@end

@implementation KIODBStore {
    keen_io_sqlite3 *keen_dbname;
    BOOL dbIsOpen;
    
    // Keen Event SQL Statements
    keen_io_sqlite3_stmt *insert_event_stmt;
    keen_io_sqlite3_stmt *find_event_stmt;
    keen_io_sqlite3_stmt *count_all_events_stmt;
    keen_io_sqlite3_stmt *count_pending_events_stmt;
    keen_io_sqlite3_stmt *make_pending_event_stmt;
    keen_io_sqlite3_stmt *reset_pending_events_stmt;
    keen_io_sqlite3_stmt *purge_events_stmt;
    keen_io_sqlite3_stmt *delete_event_stmt;
    keen_io_sqlite3_stmt *delete_all_events_stmt;
    keen_io_sqlite3_stmt *increment_event_attempts_statement;
    keen_io_sqlite3_stmt *delete_too_many_attempts_events_statement;
    keen_io_sqlite3_stmt *age_out_events_stmt;
    
    // Keen Query SQL Statements
    keen_io_sqlite3_stmt *insert_query_stmt;
    keen_io_sqlite3_stmt *count_all_queries_stmt;
    keen_io_sqlite3_stmt *get_query_stmt;
    keen_io_sqlite3_stmt *get_query_with_attempts_stmt;
    keen_io_sqlite3_stmt *increment_query_attempts_statement;
    keen_io_sqlite3_stmt *delete_all_queries_stmt;
    keen_io_sqlite3_stmt *age_out_queries_stmt;
    
    keen_io_sqlite3_stmt *convert_date_stmt;
}

- (instancetype)init {
    self = [super init];

    if(self) {
        dbIsOpen = NO;
        [self openAndInitDB];
    }
    return self;
}

# pragma mark - Handle Database -

# pragma mark Database Methods

- (BOOL)openDB {
    __block BOOL wasOpened = NO;
    NSString *libraryPath = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString *my_sqlfile = [libraryPath stringByAppendingPathComponent:@"keenEvents.sqlite"];
    
    // we're going to use a queue for all database operations, so let's create it
    self.dbQueue = dispatch_queue_create("io.keen.sqlite", DISPATCH_QUEUE_SERIAL);
    
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        // initialize sqlite ourselves so we can config
        keen_io_sqlite3_shutdown();
        keen_io_sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
        keen_io_sqlite3_initialize();
        
        int openDBResult = keen_io_sqlite3_open([my_sqlfile UTF8String], &keen_dbname);
        if (openDBResult != SQLITE_OK) {
            if(openDBResult == SQLITE_CORRUPT) {
                // delete corrupt database file
                NSError *error = nil;
                NSLog(@"%@", my_sqlfile);
                [[NSFileManager defaultManager] removeItemAtPath:my_sqlfile error:&error];
                
                // create new database file
                [self openDB];
            }
            else {
                [self handleSQLiteFailure:@"create database"];
            }
        } else {
            wasOpened = YES;
        }
        dbIsOpen = wasOpened;
    });
    
    return wasOpened;
}

- (BOOL)openAndInitDB {
    if(!dbIsOpen) {
        if (![self openDB]) {
            return false;
        } else {
            if(![self createTables]) {
                KCLog(@"Failed to create SQLite table!");
                [self closeDB];
                return false;
            }
        
            if (![self migrateTable]) {
                KCLog(@"Failed to migrate SQLite table!");
                [self closeDB];
                return false;
            }
        
            [self prepareAllSQLiteStatements];
        }
    }
    return true;
}

- (void)closeDB {
    // Free all the prepared statements. This is safe on null pointers.
    keen_io_sqlite3_finalize(insert_event_stmt);
    keen_io_sqlite3_finalize(find_event_stmt);
    keen_io_sqlite3_finalize(count_all_events_stmt);
    keen_io_sqlite3_finalize(count_pending_events_stmt);
    keen_io_sqlite3_finalize(make_pending_event_stmt);
    keen_io_sqlite3_finalize(reset_pending_events_stmt);
    keen_io_sqlite3_finalize(purge_events_stmt);
    keen_io_sqlite3_finalize(delete_event_stmt);
    keen_io_sqlite3_finalize(delete_all_events_stmt);
    keen_io_sqlite3_finalize(increment_event_attempts_statement);
    keen_io_sqlite3_finalize(delete_too_many_attempts_events_statement);
    keen_io_sqlite3_finalize(age_out_events_stmt);
    
    keen_io_sqlite3_finalize(insert_query_stmt);
    keen_io_sqlite3_finalize(count_all_queries_stmt);
    keen_io_sqlite3_finalize(get_query_stmt);
    keen_io_sqlite3_finalize(increment_query_attempts_statement);
    keen_io_sqlite3_finalize(delete_all_queries_stmt);
    keen_io_sqlite3_finalize(get_query_with_attempts_stmt);
    keen_io_sqlite3_finalize(age_out_queries_stmt);
    
    keen_io_sqlite3_finalize(convert_date_stmt);
    
    // Free our DB. This is safe on null pointers.
    keen_io_sqlite3_close(keen_dbname);
    // Reset state in case it matters.
    dbIsOpen = NO;
}

- (BOOL)createTables {
    __block BOOL wasCreated = NO;
    
    if (!dbIsOpen) {
        KCLog(@"DB is closed, skipping createTable");
        return wasCreated;
    }
    
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        //create events table
        char *eventsError;
        NSString *createEventsTableSQL = [NSString stringWithFormat:@"CREATE TABLE IF NOT EXISTS 'events' (ID INTEGER PRIMARY KEY AUTOINCREMENT, collection TEXT, projectID TEXT, eventData BLOB, pending INTEGER, dateCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"];
        if (keen_io_sqlite3_exec(keen_dbname, [createEventsTableSQL UTF8String], NULL, NULL, &eventsError) != SQLITE_OK) {
            KCLog(@"Failed to create events table: %@", [NSString stringWithCString:eventsError encoding:NSUTF8StringEncoding]);
            keen_io_sqlite3_free(eventsError); // Free that error message
            [self closeDB];
        } else {
            //create queries table
            char *queriesError;
            NSString *createQueriesTableSQL = [NSString stringWithFormat:@"CREATE TABLE IF NOT EXISTS 'queries' (ID INTEGER PRIMARY KEY AUTOINCREMENT, collection TEXT, projectID TEXT, queryData BLOB, queryType TEXT, attempts INTEGER DEFAULT 0, dateCreated TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"];
            if (keen_io_sqlite3_exec(keen_dbname, [createQueriesTableSQL UTF8String], NULL, NULL, &queriesError) != SQLITE_OK) {
                KCLog(@"Failed to create queries table: %@", [NSString stringWithCString:queriesError encoding:NSUTF8StringEncoding]);
                keen_io_sqlite3_free(queriesError); // Free that error message
                [self closeDB];
            } else {
                wasCreated = YES;
            }
        }
    });
    
    
    return wasCreated;
}

- (int)queryUserVersion {
    int databaseVersion = 0;
    
    // get current database version of schema
    static keen_io_sqlite3_stmt *stmt_version;
    
    if(keen_io_sqlite3_prepare_v2(keen_dbname, "PRAGMA user_version;", -1, &stmt_version, NULL) != SQLITE_OK) {
        return -1;
    }
    
    while(keen_io_sqlite3_step(stmt_version) == SQLITE_ROW) {
        databaseVersion = keen_io_sqlite3_column_int(stmt_version, 0);
    }
    keen_io_sqlite3_finalize(stmt_version);
    
    // -1 means error, >= 1 is a real version number, otherwise it's unversioned
    if (databaseVersion != -1 && !(databaseVersion >= 1)) {
        return 0;
    }
    
    return databaseVersion;
}

- (BOOL)setUserVersion: (int)userVersion {
    char *err;
    NSString *sql = [NSString stringWithFormat:@"PRAGMA user_version = %d;", userVersion];
    if (keen_io_sqlite3_exec(keen_dbname, [sql UTF8String], NULL, NULL, &err) != SQLITE_OK) {
        KCLog(@"failed to set user_version: %@", [NSString stringWithCString:err encoding:NSUTF8StringEncoding]);
        keen_io_sqlite3_free(err); // Free that error message
        return NO;
    }
    
    return YES;
}

- (BOOL)migrateTable {
    __block BOOL wasMigrated = NO;
    
    if (!dbIsOpen) {
        KCLog(@"DB is closed, skipping migrateTable");
        return NO;
    }
    
    // we need to wait for the queue to finish because this method has a return value
    // that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        int userVersion = [self queryUserVersion];
        KCLog(@"Preparing to migrate DB, current version: %d", userVersion);
        wasMigrated = [self migrateFromVersion:userVersion];
    });
    
    return wasMigrated;
}

-(BOOL)migrateFromVersion: (int)userVersion {
    // this is really more of a while loop, but we use a for loop with a limit to avoid
    // getting stuck in an infinite loop if there is a bug in the loop breaking logic
    for(int i = 0; i < 1000; i++) {
        if(![self beginTransaction]) {
            // deal with error?
            KCLog(@"Migration failed to begin a transaction with userVersion = %d.", userVersion);
            return NO;
        }
        
        int migrationResult = [self runMigration:userVersion];
        if (migrationResult == 0) {
            // we didn't migrate anything, because we're current.
            if (![self endTransaction]) {
                KCLog(@"Migration failed to end a transaction with userVersion = %d.", userVersion);
                return NO;
            }
            return YES;
        }
        
        if (migrationResult < 0) {
            // error
            if (![self rollbackTransaction]) {
                KCLog(@"Migration failed to rollback a transaction with userVersion = %d.", userVersion);
                // yeesh, couldn't rollback
            }
            return NO;
        }
        
        // we migrated, so increment PRAGMA user_version
        if (![self setUserVersion:userVersion+1]) {
            KCLog(@"Migration failed to set the user_version to %d.", userVersion);
            if (![self rollbackTransaction]) {
                KCLog(@"Migration failed to rollback a transaction after failing to set user_version (with userVersion = %d).", userVersion);
                // whoa, double bad news
            }
            return NO;
        }
        
        // ok, let's commit this step
        if (![self commitTransaction]) {
            KCLog(@"Migration failed to commit a transaction with userVersion = %d.", userVersion);
            return NO;
        }
        
        userVersion++;
        
        // there might be more migrations, so we loop around again
    }
    
    KCLog(@"Migration loop maxed out after 1000 iterations. This is almost certainly a bug. Version %d", [self queryUserVersion]);
    
    return NO;
}

- (int)runMigration: (int)forVersion {
    char *err;
    
    if (forVersion < 0) {
        // versions less than 0 are an error
        return -1;
    } else if (forVersion == 0) {
        // first migration does nothing, just adds adds a real version number (1).
        return YES;
    } else if (forVersion == 1) {
        NSString *sql = @"ALTER TABLE events ADD COLUMN attempts INTEGER DEFAULT 0;";
        if (keen_io_sqlite3_exec(keen_dbname, [sql UTF8String], NULL, NULL, &err) != SQLITE_OK) {
            KCLog(@"Failed to add attempts column: %@", [NSString stringWithCString:err encoding:NSUTF8StringEncoding]);
            keen_io_sqlite3_free(err); // Free that error message
            return -1;
        }
        return YES;
    } else if (forVersion == 2) {
        // This is the current version. To add a migration, increment the value of the
        // RHS of the above if statement and add another else if statement in between
        // to handle the new version number.
        // e.g. change `forVersion == 2` to `forVersion == 3`, and then add an
        // explicit block for handling the forVersion == 2 migration that looks like
        // the forVersion == 1 block above.
        
        // IMPORTANT: never remove any existing migration blocks!
        
        return 0;
    }
    
    // versions that aren't integers or are greater than the max version we know about
    // are errors
    return -1;
}

# pragma mark Transaction Methods

- (BOOL)doTransaction:(NSString *)sqlTransaction {
    char *err;
    NSString *sql = sqlTransaction;
    if (keen_io_sqlite3_exec(keen_dbname, [sql UTF8String], NULL, NULL, &err) != SQLITE_OK) {
        KCLog(@"failed to do transaction:%@, with error: %@", sqlTransaction, [NSString stringWithCString:err encoding:NSUTF8StringEncoding]);
        keen_io_sqlite3_free(err); // Free that error message
        return NO;
    }
    return YES;
}

- (BOOL)beginTransaction {
    return [self doTransaction:@"BEGIN IMMEDIATE TRANSACTION;"];
}

- (BOOL)commitTransaction {
    return [self doTransaction:@"COMMIT TRANSACTION;"];
}

- (BOOL)rollbackTransaction {
    return [self doTransaction:@"ROLLBACK TRANSACTION;"];
}

- (BOOL)endTransaction {
    return [self doTransaction:@"END TRANSACTION;"];
}

# pragma mark - Handle Events -

- (BOOL)addEvent:(NSData *)eventData collection:(NSString *)eventCollection projectID:(NSString *)projectID {
    __block BOOL wasAdded = NO;

    if(![self checkOpenDB:@"DB is closed, skipping addEvent"]) {
        return wasAdded;
    }
    
    const char *projectIDUTF8 = projectID.UTF8String;
    const char *eventCollectionUTF8 = eventCollection.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(insert_event_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to add event statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_text(insert_event_stmt, 2, eventCollectionUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind coll to add event statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_blob(insert_event_stmt, 3, [eventData bytes], (int) [eventData length], SQLITE_TRANSIENT) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind insert statement"];
            return;
        }
        
        if (keen_io_sqlite3_step(insert_event_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"insert event"];
            return;
        }
        
        wasAdded = YES;
        
        [self resetSQLiteStatement:insert_event_stmt];
    });
    
    return wasAdded;
}

- (NSMutableDictionary *)getEventsWithMaxAttempts:(int)maxAttempts andProjectID:(NSString *)projectID {
    // Create a dictionary to hold the contents of our select.
    __block NSMutableDictionary *events = [NSMutableDictionary dictionary];

    if(![self checkOpenDB:@"DB is closed, skipping getEvents"]) {
        return events;
    }
    
    // reset pending events, if necessary
    if([self hasPendingEventsWithProjectID:projectID]) {
        [self resetPendingEventsWithProjectID:projectID];
    }
    
    const char *projectIDUTF8 = projectID.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(find_event_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to find statement"];
        }
        
        if(keen_io_sqlite3_bind_int64(find_event_stmt, 2, maxAttempts) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind coll to add event statement"];
        }


        while (keen_io_sqlite3_step(find_event_stmt) == SQLITE_ROW) {
            // Fetch data out the statement
            long long eventId = keen_io_sqlite3_column_int64(find_event_stmt, 0);

            NSString *coll = [NSString stringWithUTF8String:(char *)keen_io_sqlite3_column_text(find_event_stmt, 1)];

            const void *dataPtr = keen_io_sqlite3_column_blob(find_event_stmt, 2);
            int dataSize = keen_io_sqlite3_column_bytes(find_event_stmt, 2);

            NSData *data = [[NSData alloc] initWithBytes:dataPtr length:dataSize];

            // Bind and mark the event pending.
            if(keen_io_sqlite3_bind_int64(make_pending_event_stmt, 1, eventId) != SQLITE_OK) {
                [self handleSQLiteFailure:@"bind int for make pending"];
            }
            if (keen_io_sqlite3_step(make_pending_event_stmt) != SQLITE_DONE) {
                [self handleSQLiteFailure:@"mark event pending"];
            }

            // Reset the pendifier
            [self resetSQLiteStatement:make_pending_event_stmt];

            if ([events objectForKey:coll] == nil) {
                // We don't have an entry in the dictionary yet for this collection
                // so create one.
                [events setObject:[NSMutableDictionary dictionary] forKey:coll];
            }

            [[events objectForKey:coll] setObject:data forKey:[NSNumber numberWithUnsignedLongLong:eventId]];
        }

        [self resetSQLiteStatement:find_event_stmt];
    });
    
    return events;
}

- (void)resetPendingEventsWithProjectID:(NSString *)projectID {
    if(![self checkOpenDB:@"DB is closed, skipping resetPendingEvents"]) {
        return;
    }
    
    const char *projectIDUTF8 = projectID.UTF8String;
    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(reset_pending_events_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to reset pending statement"];
            return;
        }
        if (keen_io_sqlite3_step(reset_pending_events_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"reset pending events"];
            return;
        }

        [self resetSQLiteStatement:reset_pending_events_stmt];
    });
}

- (BOOL)hasPendingEventsWithProjectID:(NSString *)projectID {
    BOOL hasRows = NO;

    if(![self checkOpenDB:@"DB is closed, skipping hasPendingEvents"]) {
        return hasRows;
    }

    if ([self getPendingEventCountWithProjectID:projectID] > 0) {
        hasRows = TRUE;
    }
    return hasRows;
}

- (NSUInteger)getPendingEventCountWithProjectID:(NSString *)projectID {
    __block NSUInteger eventCount = 0;

    if(![self checkOpenDB:@"DB is closed, skipping getPendingEventcount"]) {
        return eventCount;
    }

    const char *projectIDUTF8 = projectID.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(count_pending_events_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to count pending statement"];
            return;
        }
        if (keen_io_sqlite3_step(count_pending_events_stmt) == SQLITE_ROW) {
            eventCount = (NSInteger) keen_io_sqlite3_column_int(count_pending_events_stmt, 0);
        } else {
            [self handleSQLiteFailure:@"get count of pending rows"];
            return;
        }

        [self resetSQLiteStatement:count_pending_events_stmt];
    });
    
    return eventCount;
}

- (NSUInteger)getTotalEventCountWithProjectID:(NSString *)projectID {
    __block NSUInteger eventCount = 0;

    if(![self checkOpenDB:@"DB is closed, skipping getTotalEventCount"]) {
        return eventCount;
    }

    const char *projectIDUTF8 = projectID.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(count_all_events_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to total event statement"];
            return;
        }
        if (keen_io_sqlite3_step(count_all_events_stmt) == SQLITE_ROW) {
            eventCount = (NSInteger) keen_io_sqlite3_column_int(count_all_events_stmt, 0);
        } else {
            [self handleSQLiteFailure:@"get count of total rows"];
            return;
        }
        
        [self resetSQLiteStatement:count_all_events_stmt];
    });
    
    return eventCount;
}

- (void)deleteEvent:(NSNumber *)eventId {
    if(![self checkOpenDB:@"DB is closed, skipping deleteEvent"]) {
        return;
    }
    
    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_int64(delete_event_stmt, 1, [eventId unsignedLongLongValue]) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind eventid to delete statement"];
            return;
        }
        if (keen_io_sqlite3_step(delete_event_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"delete event"];
            return;
        };

        [self resetSQLiteStatement:delete_event_stmt];
    });
}

- (void)deleteAllEvents {
    if(![self checkOpenDB:@"DB is closed, skipping deleteEvent"]) {
        return;
    }
    
    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_step(delete_all_events_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"delete all events"];
            return;
        };
        
        [self resetSQLiteStatement:delete_all_events_stmt];
    });
}

- (void)deleteEventsFromOffset:(NSNumber *)offset {
    if(![self checkOpenDB:@"DB is closed, skipping deleteEvent"]) {
        return;
    }

    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_int64(age_out_events_stmt, 1, [offset unsignedLongLongValue]) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind offset to ageOut statement"];
            return;
        }
        if (keen_io_sqlite3_step(age_out_events_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"delete all events"];
            return;
        };

        [self resetSQLiteStatement:age_out_events_stmt];
    });
}

- (void)incrementEventUploadAttempts:(NSNumber *)eventId {
    if(![self checkOpenDB:@"DB is closed, skipping incrementAttempts"]) {
        return;
    }

    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_int64(increment_event_attempts_statement, 1, [eventId unsignedLongLongValue]) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind eventid to increment attempts statement"];
            return;
        }
        if (keen_io_sqlite3_step(increment_event_attempts_statement) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"increment attempts"];
            return;
        };

        [self resetSQLiteStatement:increment_event_attempts_statement];
    });
}

- (void)purgePendingEventsWithProjectID:(NSString *)projectID {
    if(![self checkOpenDB:@"DB is closed, skipping purgePendingEvents"]) {
        return;
    }

    const char *projectIDUTF8 = [projectID UTF8String];
    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(purge_events_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to purge statement"];
            return;
        }
        if (keen_io_sqlite3_step(purge_events_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"purge pending events"];
            return;
        };

        [self resetSQLiteStatement:purge_events_stmt];
    });
}

- (id)convertNSDateToISO8601:(id)date {
    __block NSString *iso8601 = nil;
    
    if(![self checkOpenDB:@"DB is closed, skipping date conversion"]) {
        return iso8601;
    }
    
    double offset = 0.0f;
    if([date isKindOfClass:[NSDate class]]) {
        offset = [[NSTimeZone localTimeZone] secondsFromGMTForDate:date] / 3600.00;  // need the offset
    }
    else if([date isKindOfClass:[NSString class]]) {
        offset = [[NSTimeZone localTimeZone] secondsFromGMT] / 3600.00;  // need the offset
    }
    NSArray *offsetArray = [[NSString stringWithFormat:@"%f",offset] componentsSeparatedByString:@"."]; // split it so we don't have to do math or numberformatting, which isn't thread-safe either
    NSString *hour = [[offsetArray objectAtIndex:0] stringByReplacingOccurrencesOfString:@"-" withString:@""];
    NSString *minute = [[offsetArray objectAtIndex:1] substringToIndex:2];
    
    // ensure we have leading zeros where necessary
    while([hour length] < 2) {
        hour = [@"0" stringByAppendingString:hour];
    }
    
    // minute math
    if([minute isEqual: @"25"]) { minute = @"15"; }
    if([minute isEqual: @"50"]) { minute = @"30"; }
    if([minute isEqual: @"75"]) { minute = @"45"; }
    
    NSString *offsetString = [[hour stringByAppendingString:@":"] stringByAppendingString:minute];
    
    // are we + or -?
    if(offset >= 0) {
        offsetString = [@"+" stringByAppendingString:offsetString];
    } else {
        offsetString = [@"-" stringByAppendingString:offsetString];
    }
    
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(convert_date_stmt, 1, [[NSString stringWithFormat:@"%f", [date timeIntervalSince1970]] UTF8String], -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind date to date conversion statement"];
            return;
        }
        
        if (keen_io_sqlite3_step(convert_date_stmt) != SQLITE_ROW) {
            [self handleSQLiteFailure:@"date conversion"];
            return;
        }
        
        iso8601 = [[NSString stringWithUTF8String:(char *)keen_io_sqlite3_column_text(convert_date_stmt, 0)] stringByAppendingString:offsetString];
        
        [self resetSQLiteStatement:convert_date_stmt];
    });
    
    return iso8601;
}

# pragma makr - Handle Queries

- (BOOL)addQuery:(NSData *)queryData queryType:(NSString *)queryType collection:(NSString *)eventCollection projectID:(NSString *)projectID {
    __block BOOL wasAdded = NO;

    if(![self checkOpenDB:@"DB is closed, skipping addQuery"]) {
        return wasAdded;
    }

    const char *projectIDUTF8 = projectID.UTF8String;
    const char *eventCollectionUTF8 = eventCollection.UTF8String;
    const char *queryTypeUTF8 = queryType.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(insert_query_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to add event statement"];
            return;
        }

        if (keen_io_sqlite3_bind_text(insert_query_stmt, 2, eventCollectionUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind coll to add event statement"];
            return;
        }

        if (keen_io_sqlite3_bind_blob(insert_query_stmt, 3, [queryData bytes], (int) [queryData length], SQLITE_TRANSIENT) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind insert statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_text(insert_query_stmt, 4, queryTypeUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind query type to add event statement"];
            return;
        }

        if (keen_io_sqlite3_step(insert_query_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"insert query"];
            return;
        }

        wasAdded = YES;
        
        [self resetSQLiteStatement:insert_query_stmt];
    });

    return wasAdded;
}

- (NSMutableDictionary *)getQuery:(NSData *)queryData queryType:(NSString *)queryType collection:(NSString *)eventCollection projectID:(NSString *)projectID {
    // Create a dictionary to hold the contents of our select.
    __block NSMutableDictionary *query = nil;
    
    if(![self checkOpenDB:@"DB is closed, skipping getQuery"]) {
        return query;
    }
    
    const char *projectIDUTF8 = projectID.UTF8String;
    const char *eventCollectionUTF8 = eventCollection.UTF8String;
    const char *queryTypeUTF8 = queryType.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(get_query_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to get query statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_text(get_query_stmt, 2, eventCollectionUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind collection to get query statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_blob(get_query_stmt, 3, [queryData bytes], (int)[queryData length], SQLITE_TRANSIENT) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind query data to get query statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_text(get_query_stmt, 4, queryTypeUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind query type to get query statement"];
            return;
        }
        
        if (keen_io_sqlite3_step(get_query_stmt) == SQLITE_ROW) {
            // Fetch data out the statement
            query = [NSMutableDictionary dictionary];
            
            NSNumber *queryID = [NSNumber numberWithUnsignedLongLong:keen_io_sqlite3_column_int64(get_query_stmt, 0)];
            
            NSString *eventCollection = [NSString stringWithUTF8String:(char *)keen_io_sqlite3_column_text(get_query_stmt, 1)];
            
            const void *dataPtr = keen_io_sqlite3_column_blob(get_query_stmt, 2);
            int dataSize = keen_io_sqlite3_column_bytes(get_query_stmt, 2);
            
            NSData *data = [[NSData alloc] initWithBytes:dataPtr length:dataSize];
            
            NSString *queryType = [NSString stringWithUTF8String:(char *)keen_io_sqlite3_column_text(get_query_stmt, 3)];
            
            NSNumber *attempts = [NSNumber numberWithUnsignedLong:keen_io_sqlite3_column_int(get_query_stmt, 4)];
            
            [query setObject:queryID forKey:@"queryID"];
            [query setObject:eventCollection forKey:@"event_collection"];
            [query setObject:data forKey:@"queryData"];
            [query setObject:queryType forKey:@"queryType"];
            [query setObject:attempts forKey:@"attempts"];
        } else {
            [self handleSQLiteFailure:@"find query"];
            return;
        }
        
        [self resetSQLiteStatement:get_query_stmt];
    });
    
    return query;
}

- (BOOL)incrementQueryAttempts:(NSNumber *)queryID {
    __block BOOL wasUpdated = NO;
    
    if(![self checkOpenDB:@"DB is closed, skipping query increment attempt"]) {
        return wasUpdated;
    }
    
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_int64(increment_query_attempts_statement, 1, [queryID unsignedLongLongValue]) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind eventid to increment attempts statement"];
            return;
        }
        if (keen_io_sqlite3_step(increment_query_attempts_statement) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"increment attempts"];
            return;
        };
        
        wasUpdated = YES;
        
        [self resetSQLiteStatement:increment_query_attempts_statement];
    });
    
    return wasUpdated;
}

- (void)findOrUpdateQuery:(NSData *)queryData queryType:(NSString *)queryType collection:(NSString *)eventCollection projectID:(NSString *)projectID {
    NSMutableDictionary *returnedQuery = [self getQuery:queryData queryType:queryType collection:eventCollection projectID:projectID];
    if(returnedQuery != nil) {
        // if query is found, update query attempts
        [self incrementQueryAttempts:[returnedQuery objectForKey:@"queryID"]];
    } else {
        // else add it to the database
        [self addQuery:queryData queryType:queryType collection:eventCollection projectID:projectID];
    }
}

- (NSUInteger)getTotalQueryCountWithProjectID:(NSString *)projectID {
    __block NSUInteger queryCount = 0;
    
    if(![self checkOpenDB:@"DB is closed, skipping getTotalQueryCount"]) {
        return queryCount;
    }

    const char *projectIDUTF8 = projectID.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(count_all_queries_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to total query statement"];
            return;
        }
        if (keen_io_sqlite3_step(count_all_queries_stmt) == SQLITE_ROW) {
            queryCount = (NSInteger) keen_io_sqlite3_column_int(count_all_queries_stmt, 0);
        } else {
            [self handleSQLiteFailure:@"get count of total query rows"];
            return;
        }
        
        [self resetSQLiteStatement:count_all_queries_stmt];
    });

    return queryCount;
}

- (BOOL)hasQueryWithMaxAttempts:(NSData *)queryData
                      queryType:(NSString *)queryType
                     collection:(NSString *)eventCollection
                      projectID:(NSString *)projectID
                    maxAttempts:(int)maxAttempts
                       queryTTL:(int)queryTTL {
    
    __block BOOL hasFoundEventWithMaxAttempts = NO;
    
    if(![self checkOpenDB:@"DB is closed, skipping hasQueryWithMaxAttempts"]) {
        return hasFoundEventWithMaxAttempts;
    }
    
    // clear query database based on timespan
    [self deleteQueriesOlderThan:[NSNumber numberWithInt:queryTTL]];
    
    const char *projectIDUTF8 = projectID.UTF8String;
    const char *eventCollectionUTF8 = eventCollection.UTF8String;
    const char *queryTypeUTF8 = queryType.UTF8String;
    // we need to wait for the queue to finish because this method has a return value that we're manipulating in the queue
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(get_query_with_attempts_stmt, 1, projectIDUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind pid to has query with max attempts statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_text(get_query_with_attempts_stmt, 2, eventCollectionUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind collection to has query with max attempts statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_blob(get_query_with_attempts_stmt, 3, [queryData bytes], (int)[queryData length], SQLITE_TRANSIENT) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind query data to has query with max attempts statement"];
            return;
        }
        
        if (keen_io_sqlite3_bind_text(get_query_with_attempts_stmt, 4, queryTypeUTF8, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind query type to has query with max attempts statement"];
            return;
        }
        
        if(keen_io_sqlite3_bind_int64(get_query_with_attempts_stmt, 5, maxAttempts) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind attempts to has query with max attempts statement"];
        }
        
        if (keen_io_sqlite3_step(get_query_with_attempts_stmt) == SQLITE_ROW) {
            hasFoundEventWithMaxAttempts = YES;
        } else {
            [self handleSQLiteFailure:@"find query with max attempts"];
            return;
        }
        
        [self resetSQLiteStatement:get_query_with_attempts_stmt];
    });
    
    return hasFoundEventWithMaxAttempts;
}

- (void)deleteAllQueries {
    if(![self checkOpenDB:@"DB is closed, skipping deleteAllQueries"]) {
        return;
    }
    
    dispatch_async(self.dbQueue, ^{
        if (keen_io_sqlite3_step(delete_all_queries_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"delete all queries"];
            return;
        };
        
        [self resetSQLiteStatement:delete_all_queries_stmt];
    });
}

- (void)deleteQueriesOlderThan:(NSNumber *)seconds {
    if(![self checkOpenDB:@"DB is closed, skipping deleteQueries"]) {
        return;
    }
    
    const char *secondsSQLUTF8String = [[NSString stringWithFormat:@"-%@ seconds", seconds] UTF8String];
    dispatch_sync(self.dbQueue, ^{
        if (keen_io_sqlite3_bind_text(age_out_queries_stmt, 1, secondsSQLUTF8String, -1, SQLITE_STATIC) != SQLITE_OK) {
            [self handleSQLiteFailure:@"bind seconds to delete query statement"];
            return;
        }
        
        if (keen_io_sqlite3_step(age_out_queries_stmt) != SQLITE_DONE) {
            [self handleSQLiteFailure:@"delete old queries"];
            return;
        };
        
        [self resetSQLiteStatement:age_out_queries_stmt];
    });
}

# pragma mark - Helper Methods -

- (BOOL)checkOpenDB:(NSString *)failureMessage {
    if(![self openAndInitDB]) {
        KCLog(@"%@", failureMessage);
        return false;
    }
    
    return true;
}

- (void)prepareSQLStatement:(keen_io_sqlite3_stmt **)sqlStatement sqlQuery:(char *)sqlQuery failureMessage:(NSString *)failureMessage {
    if(keen_io_sqlite3_prepare_v2(keen_dbname, sqlQuery, -1, sqlStatement, NULL) != SQLITE_OK) {
        [self handleSQLiteFailure:failureMessage];
    }
}

- (void)resetSQLiteStatement:(keen_io_sqlite3_stmt *)sqliteStatement {
    keen_io_sqlite3_reset(sqliteStatement);
    keen_io_sqlite3_clear_bindings(sqliteStatement);
}

- (void)prepareAllSQLiteStatements {

    // EVENT STATEMENTS
    
    // This statement inserts events into the table.
    [self prepareSQLStatement:&insert_event_stmt sqlQuery:"INSERT INTO events (projectID, collection, eventData, pending, attempts) VALUES (?, ?, ?, 0, 0)" failureMessage:@"prepare insert event statement"];
    
    // This statement finds non-pending events in the table.
    [self prepareSQLStatement:&find_event_stmt sqlQuery:"SELECT id, collection, eventData FROM events WHERE pending=0 AND projectID=? AND attempts<?" failureMessage:@"prepare find non-pending events statement"];
    
    // This statement counts the total number of events (pending or not)
    [self prepareSQLStatement:&count_all_events_stmt sqlQuery:"SELECT count(*) FROM events WHERE projectID=?" failureMessage:@"prepare count all events statement"];
    
    // This statement counts the number of pending events.
    [self prepareSQLStatement:&count_pending_events_stmt sqlQuery:"SELECT count(*) FROM events WHERE pending=1 AND projectID=?" failureMessage:@"prepare count pending events statement"];
    
    // This statement marks an event as pending.
    [self prepareSQLStatement:&make_pending_event_stmt sqlQuery:"UPDATE events SET pending=1 WHERE id=?" failureMessage:@"prepare mark event as pending statement"];
    
    // This statement resets pending events back to normal.
    [self prepareSQLStatement:&reset_pending_events_stmt sqlQuery:"UPDATE events SET pending=0 WHERE projectID=?" failureMessage:@"prepare reset pending statement"];
    
    // This statement purges all pending events.
    [self prepareSQLStatement:&purge_events_stmt sqlQuery:"DELETE FROM events WHERE pending=1 AND projectID=?" failureMessage:@"prepare purge pending events statement"];
    
    // This statement deletes a specific event.
    [self prepareSQLStatement:&delete_event_stmt sqlQuery:"DELETE FROM events WHERE id=?" failureMessage:@"prepare delete specific event statement"];
    
    // This statement deletes all events.
    [self prepareSQLStatement:&delete_all_events_stmt sqlQuery:"DELETE FROM events" failureMessage:@"prepare delete all events statement"];
    
    // This statement deletes old events at a given offset.
    [self prepareSQLStatement:&age_out_events_stmt sqlQuery:"DELETE FROM events WHERE id <= (SELECT id FROM events ORDER BY id DESC LIMIT 1 OFFSET ?)" failureMessage:@"prepare delete old events at offset statement"];
    
    // This statement increments the attempts count of an event.
    [self prepareSQLStatement:&increment_event_attempts_statement sqlQuery:"UPDATE events SET attempts = attempts + 1 WHERE id=?" failureMessage:@"prepare event increment attempt statement"];
    
    // This statement deletes events exceeding a max attempt limit.
    [self prepareSQLStatement:&delete_too_many_attempts_events_statement sqlQuery:"DELETE FROM events WHERE attempts >= ?" failureMessage:@"prepare delete max attempts events statement"];

    
    // QUERY STATEMENTS
    
    // This statement inserts queries into the table.
    [self prepareSQLStatement:&insert_query_stmt sqlQuery:"INSERT INTO queries (projectID, collection, queryData, queryType, attempts) VALUES (?, ?, ?, ?, 0)" failureMessage:@"prepare insert query statement"];
    
    // This statement counts the total number of queries
    [self prepareSQLStatement:&count_all_queries_stmt sqlQuery:"SELECT count(*) FROM queries WHERE projectID=?" failureMessage:@"prepare count all queries statement"];
 
    // This statement searches for and returns a query.
    [self prepareSQLStatement:&get_query_stmt sqlQuery:"SELECT id, collection, queryData, queryType, attempts FROM queries WHERE projectID=? AND collection=? AND queryData=? AND queryType=?" failureMessage:@"prepare find query statement"];
    
    // This statement searches for and returns a query given an attempts value.
    [self prepareSQLStatement:&get_query_with_attempts_stmt sqlQuery:"SELECT id FROM queries WHERE projectID=? AND collection=? AND queryData=? AND queryType=? AND attempts >=?" failureMessage:@"prepare find query with attempts statement"];
    
    // This statement increments the attempts count of a query.
    [self prepareSQLStatement:&increment_query_attempts_statement sqlQuery:"UPDATE queries SET attempts = attempts + 1 WHERE id=?" failureMessage:@"prepare query increment attempt statement"];
    
    // This statement deletes all queries.
    [self prepareSQLStatement:&delete_all_queries_stmt sqlQuery:"DELETE FROM queries" failureMessage:@"prepare delete all queries statement"];
    
    // This statement deletes old queries at a given time.
    [self prepareSQLStatement:&age_out_queries_stmt sqlQuery:"DELETE FROM queries WHERE dateCreated <= datetime('now', ?)" failureMessage:@"prepare delete old queries at seconds statement"];
    
    // HELPER STATEMENTS
    
    // This statement converts an NSDate to an ISO-8601 formatted date/time string (we use sqlite because NSDateFormatter isn't thread-safe)
    [self prepareSQLStatement:&convert_date_stmt sqlQuery:"SELECT strftime('%Y-%m-%dT%H:%M:%S',datetime(?,'unixepoch','localtime'))" failureMessage:@"prepare convert date statement"];
}

- (void)handleSQLiteFailure:(NSString *) msg {
    KCLog(@"Failed to %@: %@",
          msg, [NSString stringWithCString:keen_io_sqlite3_errmsg(keen_dbname) encoding:NSUTF8StringEncoding]);
    [self closeDB];
}

@end
