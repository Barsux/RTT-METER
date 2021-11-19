#ifndef coreH
#define coreH

#include "base.h"
#include "l2_transport.h"
#include "mgmt.h"
#include "file_mgmt.h"

class Core {public:
  virtual ~Core() {}
  class Setup {public:
  };

  virtual void attach_l2_transport(
    L2Transport::Queue_rx*   l2_transport_rx,
    L2Transport::Queue_tx*   l2_transport_tx,
    L2Transport::Queue_sent* sent
  ) = 0;

  virtual void attach_mgmt(
    Mgmt::Queue_job* mgmt_job,
    Mgmt::Queue_report* mgmt_report
  ) = 0;

  virtual void attach_file_mgmt(
    File_mgmt::Queue_toSet* fmgmt_set,
    File_mgmt::Queue_toSave* fmgmt_save
  ) = 0;


};

Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup);


#endif
