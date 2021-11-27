#ifndef l2_transportH
#define l2_transportH

#include "base.h"

class L2Transport {public:
  virtual ~L2Transport() {}
  class Setup {public:
    char* physicalId;
    Setup(): physicalId() {}
  };
  class Queue_rx: public WaitSystem::Queue {public:
    virtual int recv(struct msghdr &msg) = 0;
  }* rx;
  class Queue_tx: public WaitSystem::Queue {public:
    virtual int send(struct msghdr msg) = 0;
  }* tx;
  class Queue_sent: public WaitSystem::Queue {public:
    U64 utc_sent;
  }* sent;
};

#endif
