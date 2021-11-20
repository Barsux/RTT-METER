#ifndef coreH
#define coreH

#include "base.h"
#include "l2_transport_linux.h"
#include "mgmt_linux.h"
#include "global_setup_linux.h"
#include "packager_linux.h"

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

  //virtual void attach_Global_setup(
  //  Global_setup::Queue_toSet* setup_set,
  //  Global_setup::Queue_toSave* setup_save
  //) = 0;

  virtual void attach_packager(
    Packager::Queue_prx* packager_rx,
    Packager::Queue_ptx* packager_tx,
    Packager::Queue_psent* packager_sent
  ) = 0;

};

Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup);


#endif
