
#ifndef MGMT_H
#define MGMT_H

#include "base.h"
#include "file_mgmt.h"

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
    virtual void attach_file_mgmt(
        File_mgmt::Queue_toSet* fmgmt_set,
        File_mgmt::Queue_toSave* fmgmt_save
    ) = 0;
};

Mgmt* new_Mgmt(WaitSystem* waitSystem, Mgmt::Setup &setup);

#endif MGMT_H