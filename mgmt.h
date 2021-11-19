
#ifndef MGMT_H
#define MGMT_H

#include "base.h"

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
};

Mgmt* new_Mgmt(WaitSystem* waitSystem, Mgmt::Setup &setup);

#endif MGMT_H