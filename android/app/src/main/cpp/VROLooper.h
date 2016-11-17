//
//  VROLooper.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <pthread.h>
#include <semaphore.h>

struct loopermessage;

class VROLooper {
    public:
        VROLooper();
        VROLooper& operator=(const VROLooper& ) = delete;
        VROLooper(VROLooper&) = delete;
        virtual ~VROLooper();

        void post(int what, void *data, bool flush = false);
        virtual void quit();

        virtual void handle(int what, void *data);

    private:
        void addmsg(loopermessage *msg, bool flush);
        static void* trampoline(void* p);
        void loop();
        loopermessage *head;
        pthread_t worker;
        sem_t headwriteprotect;
        sem_t headdataavailable;
        bool running;
};
