#include "base.h"
#include "core.h"
#include "l2_transport_linux.h"
#include "mgmt.h"
#include "file_mgmt.h"

#pragma argsused
int main(int argc, char **argv)
{
  WaitSystem* waitSystem = new_WaitSystem();

  Core::Setup coreSetup;
  Core* core = new_Core(waitSystem, coreSetup);

  Mgmt::Setup mgmtSetup;
  memcpy(mgmtSetup.argv, argv, sizeof(argv));
  mgmtSetup.argc = argc;
  Mgmt* mgmt = new_Mgmt(waitSystem, mgmtSetup);

  L2Transport::Setup l2Transport_setup;
  l2Transport_setup.physicalId = "enp4s0";
  L2Transport* l2Transport = new_L2Transport(waitSystem, l2Transport_setup);

  File_mgmt::Setup file_mgmt_setup;
  file_mgmt_setup.path = "PATH_TO_CFG";

  core->attach_mgmt(mgmt->job, mgmt->report);
  core->attach_l2_transport(l2Transport->rx, l2Transport->tx, l2Transport->sent);


  waitSystem->run();
}


