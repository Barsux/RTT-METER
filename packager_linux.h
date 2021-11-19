#ifndef RTT_METER_PACKAGER_LINUX_H
#define RTT_METER_PACKAGER_LINUX_H

#include "packager.h"

Packager* new_Packager(WaitSystem* waitSystem, Packager::Setup &setup);
#endif
