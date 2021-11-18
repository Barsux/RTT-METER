#include "base.h"
#include "core.h"
#include "l2_transport_linux.h"
#include "mgmt.h"

#pragma argsused
int main(int argc, char **argv)
{
  WaitSystem* waitSystem = new_WaitSystem();
  Core::Setup coreSetup;
  Core* core = new_Core(waitSystem, coreSetup);

  Mgmt::Setup mgmtSetup;
  mgmtSetup.argc = argc;
  mgmtSetup.argv = argv;
  Mgmt* mgmt = new_Mgmt(waitSystem, mgmtSetup);
  core->attach_mgmt(mgmt->job, mgmt->report);

  L2Transport::Setup l2Transport_setup;
  l2Transport_setup.physicalId = "enp4s0";
  L2Transport* l2Transport = new_L2Transport(waitSystem, l2Transport_setup);
  core->attach_l2_transport(l2Transport->rx, l2Transport->tx, l2Transport->sent);


  waitSystem->run();
}


