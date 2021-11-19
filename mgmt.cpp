#include "mgmt.h"

class MgmtObject: public WaitSystem::Module, public Mgmt {
    Mgmt::Setup &setup;
public:
    bool converted;
    class Job: public Queue_job {public:
        MgmtObject &base;
        Job(MgmtObject &base): base(base){}
        struct pckt convert(int argc, char **argv) {
            return base.convert(argc,  argv);
        }
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
        module_debug = "Managment";
        job = &mgmt_job;
        report = &mgmt_report; enable_wait(report);
    }
    void report_void(){

    }
    struct pckt convert(int argc, char **argv){
        struct pckt data;
        print("Converted!");
        return data;
    }
    void evaluate(){
        if(!converted) {
            job->packet = convert(setup.argc, setup.argv);
            job->setReady();
        }
    }
};

Mgmt* new_Mgmt(WaitSystem* waitSystem, Mgmt::Setup &setup){
    return new MgmtObject(waitSystem, setup);
}
