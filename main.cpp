#include "base.h"
#include "core.h"
#include "l2_transport_linux.h"
#include "mgmt_linux.h"
#include "global_setup_linux.h"
#include "packetizer_linux.h"

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

  Global_setup* global_setup = new_Global_setup(waitSystem);

  Packetizer::Setup packager_setup;
  Packetizer* packetizer = new_Packetizer(waitSystem, packager_setup);
  packetizer->attach_l2_transport(l2Transport->rx, l2Transport->tx, l2Transport->sent);
  packetizer->attach_Global_setup(global_setup->save, global_setup->set);

  mgmt->attach_Global_setup(global_setup->save, global_setup->set);


  core->attach_Global_setup(global_setup->save, global_setup->set);
  core->attach_mgmt(mgmt->job, mgmt->report);
  core->attach_packetizer(packetizer->rx, packetizer->tx, packetizer->sent);


  waitSystem->run();
}


