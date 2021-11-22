#ifndef PACKETIZER_H
#define PACKETIZER_H

#include "base.h"
#include "l2_transport_linux.h"
#include "global_setup_linux.h"


#pragma pack(push, 1)
struct ethheader{
    MAC dst;
    MAC src;
    U16 protocol;
};

struct ipheader {
    unsigned char   ihl:4,ver:4;
    unsigned char           tos;
    unsigned short int      len;
    unsigned short int    ident;
    unsigned char          flag;
    unsigned short int   offset;
    unsigned char           ttl;
    unsigned char      protocol;
    unsigned short int    check;
    IP4                sourceip;
    IP4                  destip;

};

struct udpheader {
    unsigned short int src;
    unsigned short int dst;
    unsigned short int len;
    unsigned short int check;

};
#pragma pack(pop)


class Packetizer{public:
    virtual ~Packetizer() {}
    class Setup {public:
    };
    class Queue_prx: public WaitSystem::Queue {public:
    }* rx;
    class Queue_ptx: public WaitSystem::Queue {public:
        virtual int send(int seq) = 0;
    }* tx;
    class Queue_psent: public WaitSystem::Queue {public:
    }* sent;
    virtual void attach_l2_transport(
            L2Transport::Queue_rx*   l2_transport_rx,
            L2Transport::Queue_tx*   l2_transport_tx,
            L2Transport::Queue_sent* sent
    ) = 0;

    virtual void attach_Global_setup(
            Global_setup::Queue_toSave* setup_save,
            Global_setup::Queue_toSet* setup_set
    ) = 0;
};


#endif PACKETIZER_H
