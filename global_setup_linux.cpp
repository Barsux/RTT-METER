#include "global_setup_linux.h"

class Global_setupObject: public WaitSystem::Module, public Global_setup{
public:
    char * path;
    bool setted;
    class Set : public Queue_toSet {public:
        Global_setupObject &base;
        Set(Global_setupObject &base): base(base){}
        struct settings get_values(char * dir){
            return base.get_settings(dir);
        }
    } setup_set;

    class Save : public Queue_toSave {public:
        Global_setupObject &base;
        Save(Global_setupObject &base): base(base){}
        int save_values(struct settings config){
            return base.save_values(config);
        }
    } setup_save;

    Global_setupObject(WaitSystem* waitSystem): WaitSystem::Module(waitSystem)
            , setted(false), path("cfg.json"), setup_set(*this), setup_save(*this)
    {
        module_debug = "File_Managment";
        save = &setup_save;
        set = &setup_set;
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
        }
    }
};
Global_setup* new_Global_setup(WaitSystem* waitSystem){
    return new Global_setupObject(waitSystem);
}
