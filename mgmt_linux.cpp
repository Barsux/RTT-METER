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
        struct pckt convert(int argc, char **argv) {
            return base.convert(argc,  argv);
        }
    } mgmt_job;

    class Report: public Queue_report {public:
        MgmtObject &base;
        Report(MgmtObject &base): base(base){}
        void report(long measure[60000], int amount){
            return base.report_void(measure, amount);
        }
    } mgmt_report;
    MgmtObject(WaitSystem* waitSystem, Mgmt::Setup &setup): WaitSystem::Module(waitSystem)
            , setup(setup), converted(false), mgmt_job(*this), mgmt_report(*this)
    {
        module_debug = "Managment";
        job = &mgmt_job;
        report = &mgmt_report; enable_wait(report);
    }

    void attach_Global_setup(Global_setup::Queue_toSave* save, Global_setup::Queue_toSet* set){
        setup_set = set; setup_save = save;
        disable_wait(setup_set); disable_wait(setup_save);
        enable_wait(setup_set);
    }

    void report_void(long measure[60000], int amount){
        report->clear();
        long long int avg_rtt = 0;
        long long int max_rtt = 0;
        int loss_packets = 0;
        for(int i = 0; i < amount; i++){
            if(measure[i] != 0) {
                avg_rtt += (int)measure[i];
                if (measure[i] > max_rtt) max_rtt = (int)measure[i];
                print("");
                print("%lli", max_rtt);
                print("");
            }
            else{
                loss_packets++;
            }
        }
        print("%li\t%li", avg_rtt, max_rtt);
        long double avg_output = 0, max_output = 0;
        avg_rtt = ((long double)avg_rtt) / 1000000;
        print("%0.4f\t%0.4f", avg_rtt, max_rtt);
        exit(EXIT_SUCCESS);
    }
    struct pckt convert(int argc, char **argv){
        struct pckt data;
        print("Converted!");
        if(argc == 8){
            str2mac(data.srcMAC, argv[1]);
            str2mac(data.dstMAC, argv[2]);
            data.srcIP = inet_addr(argv[3]);
            data.dstIP = inet_addr(argv[4]);
            str2int(data.size, argv[5]);
            str2int(data.pckt_per_s, argv[6]);
            str2int(data.duration, argv[7]);
            data.amount = data.duration * data.pckt_per_s;
        }
        else{
            print("I am server!");
            data.is_server = true;
        }
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