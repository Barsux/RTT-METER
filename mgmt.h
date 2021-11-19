
#ifndef MGMT_H
#define MGMT_H

#include "base.h"
#include "global_setup.h"

class Mgmt{public:
    struct pckt{
        MAC srcMAC, dstMAC;
        IP4 srcIP, dstIP;
        int scrPORT, dstPORT;
        int size, pckt_per_s, duration;
        pckt(): scrPORT(5850), dstPORT(5850), size(1024), pckt_per_s(1), duration(1) {}
    }pckt;
    virtual ~Mgmt() {}
    class Setup {public:
        int argc;
        char* argv[];
    };
    class Queue_job: public WaitSystem::Queue {public:
        struct pckt packet;
    }* job;
    class Queue_report: public WaitSystem::Queue {public:
    }* report;
    virtual void attach_Global_setup(
            Global_setup::Queue_toSet* setup_set,
            Global_setup::Queue_toSave* setup_save
    ) = 0;
};


#endif MGMT_H