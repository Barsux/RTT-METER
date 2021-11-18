
#ifndef MGMT_H
#define MGMT_H

#include "base.h"

class Mgmt{
    virtual ~Mgmt() {}
    class Setup {public:
        int argc;
        char* argv[];
    };
    class Queue_job: public WaitSystem::Queue {public:
    }* job;
    class Queue_report: public WaitSystem::Queue {public:
    }* report;
};

Mgmt* new_Mgmt(WaitSystem* waitSystem, Mgmt::Setup &setup);

#endif MGMT_H