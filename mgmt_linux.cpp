#include "mgmt_linux.h"

class MgmtObject: public WaitSystem::Module, public Mgmt {
    Mgmt::Setup &setup;
public:

    Global_setup::Queue_toSet*       setup_set;
    Global_setup::Queue_toSave*     setup_save;

    bool converted;
    class Job: public Queue_job {public:
        MgmtObject &base;
        Job(MgmtObject &base): base(base){}
        struct pckt packet;
    } mgmt_job;

    class Report: public Queue_report {public:
        MgmtObject &base;
        Report(MgmtObject &base): base(base){}
        void report(){
            return base.report_void();
        }
    } mgmt_report;
    MgmtObject(WaitSystem* waitSystem, Mgmt::Setup &setup): WaitSystem::Module(waitSystem)
            , setup(setup), converted(false), mgmt_job(*this), mgmt_report(*this)
    {
        module_debug = "MGMT";
        job = &mgmt_job;
        report = &mgmt_report; enable_wait(report);
    }

    void attach_Global_setup(Global_setup::Queue_toSave* save, Global_setup::Queue_toSet* set){
        setup_set = set; setup_save = save;
        disable_wait(setup_set); disable_wait(setup_save);
        enable_wait(setup_set);
    }

    void report_void(){

    }
    struct pckt convert(int argc, char **argv){
        struct pckt data;
        if(argc == 7){
            data.is_server = false;
            str2mac(data.srcMAC, argv[1]);
            str2mac(data.dstMAC, argv[2]);
            data.srcIP = inet_addr(argv[3]);
            data.dstIP = inet_addr(argv[4]);
            str2int(data.size, argv[5]);
            str2int(data.pckt_per_s, argv[6]);
            str2int(data.duration, argv[7]);
        }
        else{
            data.is_server = true;
        }
        return data;
    }
    void evaluate(){
        if(!converted) {
            job->packet = convert(setup.argc, setup.argv);
            job->setReady();
            print("READY!");
        }
    }


};

Mgmt* new_Mgmt(WaitSystem* waitSystem, Mgmt::Setup &setup){
    return new MgmtObject(waitSystem, setup);
}
