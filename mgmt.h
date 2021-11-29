
#ifndef MGMT_H
#define MGMT_H

#include "base.h"
#include "global_setup.h"

class Mgmt{public:
    virtual ~Mgmt() {}
    class Setup {public:
        int argc;
        char * argv[8];
    };
    class Queue_job: public WaitSystem::Queue {public:
        struct pckt packet;
    }* job;
    class Queue_report: public WaitSystem::Queue {public:
        virtual void report(long measure[60000], int amount) = 0;
    }* report;
    virtual void attach_Global_setup(
            Global_setup::Queue_toSave* setup_save,
            Global_setup::Queue_toSet* setup_set
    ) = 0;
};


#endif MGMT_H