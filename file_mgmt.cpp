#include "file_mgmt.h"

class File_mgmtObject: public WaitSystem::Module, public File_mgmt {
    File_mgmt::Setup &setup;
public:
    char * path;
    bool setted;
    class Set: public Queue_toSet {public:
        File_mgmtObject &base;
        Set(File_mgmtObject &base): base(base){}
        struct settings get_values(char * dir){
            return base.get_settings(dir);
        }
    } fmgmt_set;

    class Save: public Queue_toSave {public:
        File_mgmtObject &base;
        Save(File_mgmtObject &base): base(base){}
        int save_values(struct settings config){
            return base.save_values(config);
        }
    } fmgmt_save;

    File_mgmtObject(WaitSystem* waitSystem, File_mgmt::Setup &setup): WaitSystem::Module(waitSystem)
        ,setup(setup), setted(false),path("cfg.json"), fmgmt_set(*this), fmgmt_save(*this)
    {
        module_debug = "File_Managment";
        save = &fmgmt_save;
        set = &fmgmt_set;
        enable_wait(save);
    }

    struct settings get_settings(char * path){
        struct settings config;
        return config;
    }

    int save_values(struct settings config){
        BUNG;
    }

    void evaluate(){
        if(!setted){
            if(setup.path != path) path = setup.path;
            get_settings(path);
        }
    }
};
File_mgmt* new_File_mgmt(WaitSystem* waitSystem, File_mgmt::Setup &setup){
    return new File_mgmtObject(waitSystem, setup);
}
