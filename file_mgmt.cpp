#include "file_mgmt.h"
class File_mgmtObject: public WaitSystem::Module, public File_mgmt {
    File_mgmt::Setup &setup;
public:
    bool setted;
    class Set: public Queue_toSet {public:
        File_mgmtObject &base;
        Job(File_mgmtObject &base): base(base){}
    } fmgmt_set;

    class Save: public Queue_toSave {public:
        File_mgmtObject &base;
        Report(File_mgmtObject &base): base(base){}
    } fmgmt_save;

    File_mgmtObject(WaitSystem* waitSystem, Mgmt::Setup &setup): WaitSystem::Module(waitSystem)
            , setup(setup), setted(false), fmgmt_set(*this), fmgmt_save(*this)
    {
        module_debug = "File_Managment";
        save = &fmgmt_save;
        set = &fmgmt_set;
        enable_wait(save);
    }
};

File_mgmt* new_File_mgmt(WaitSystem* waitSystem, File_mgmt::Setup &setup){
    return new File_mgmtObject(waitSystem, setup);
}
