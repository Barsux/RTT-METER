#ifndef FILE_MGMT_H
#define FILE_MGMT_H
#include "base.h"
class File_mgmt{public:
    virtual ~File_mgmt() {}
    class Setup {public:
    };
    class Queue_toSet: public WaitSystem::Queue {public:
    }* set;
    class Queue_toSave: public WaitSystem::Queue {public:
    }* save;
};

File_mgmt* new_File_mgmt(WaitSystem* waitSystem, File_mgmt::Setup &setup);




#endif FILE_MGMT_H
