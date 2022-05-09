#ifndef coreH
#define coreH

#include "base.h"
#include "packetizer.h"
#include "l2_transport.h"

struct OTT{
	U16 seq;
	TsNs out_ts;
	TsNs inc_ts;
	
};

class Core {public:
	virtual ~Core() {}
	class Setup {public:
	};
	
	virtual void attach_packetizer(
		Packetizer::Queue_prx* prx,
		Packetizer::Queue_ptx* ptx,
		Packetizer::Queue_psent* psent
	) = 0;
};

Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup);


#endif
