
#ifndef PACKAGER_H
#define PACKAGER_H

#include "base.h"
#include "l2_transport_linux.h"


class Packager{public:
    virtual ~Packager() {}
    class Setup {public:
    };
    class Queue_prx: public WaitSystem::Queue {public:
    }* rx;
    class Queue_ptx: public WaitSystem::Queue {public:
    }* tx;
    class Queue_psent: public WaitSystem::Queue {public:
    }* sent;
    virtual void attach_l2_transport(
            L2Transport::Queue_rx*   l2_transport_rx,
            L2Transport::Queue_tx*   l2_transport_tx,
            L2Transport::Queue_sent* sent
    ) = 0;
};


#endif PACKAGER_H