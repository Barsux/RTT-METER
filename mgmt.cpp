#include "mgmt.h"

class MgmtObject: public WaitSystem::Module, public Mgmt {
    Mgmt::Setup &setup;
public:
    struct pckt{
        MAC srcMAC, dstMAC;
        IP4 srcIP, dstIP;
        int scrPORT, dstPORT;
        int size, pckt_per_s, duration;
        pacekt(){
            memset(srcMAC, 0, sizeof(srcMAC));
            memset(dstMAC, 0, sizeof(dstMAC));
            memset(srcIP, 0, sizeof(srcIP));
            memset(dstIP, 0, sizeof(dstIP));
            scrPORT(), dstPORT(), size(1024), pckt_per_s(1), duration(1);
        }
    }pckt;

    bool converted;
    class Job: public Queue_job {public:
        MgmtObject &base;
        struct pckt convert(char* argv[], int argc){
            return base.convert(char* argv[], int argc);
        }
    } mgmt_job;

    class Report: public Queue_report {public:
        void report()
    } mgmt_report;
    MgmtObject(WaitSystem* waitSystem, Mgmt::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), converted(false), mgmt_job(*this), mgmt_report(*this)
    {
        module_debug = "MGMT";
        job = &mgmt_job;
        report = &mgmt_report; enable_wait(mgmt_report);
        //Сейчас разово т.к программа рассчитана на одно измерение//
    }
    pckt convert(){
        return
    }
    void evaluate(){
        if(!converted) convert();

    }
};