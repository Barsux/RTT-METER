#ifndef PACKETIZER_LINUX_H
#define PACKETIZER_LINUX_H

#include "packetizer.h"

#pragma pack(push, 1)
struct ethheader{
    UC h_source[6];
    UC h_dest[6];
    __be16 h_proto;
};

struct ipheader {
    U ihl:4;
    U version:4;
    U8 tos;
    U16 tot_len;
    U16 id;
    U16 frag_off;
    U8 ttl;
    U8 protocol;
    U16 check;
    U32 saddr;
    U32 daddr;

};

struct udpheader {
    U16 source;
    U16 dest;
    U16 len;
    U16 check;
};

struct rttheader {
    U32 sequence;
};
#pragma pack(pop)


Packetizer* new_Packetizer(WaitSystem* waitSystem, Packetizer::Setup &setup);
#endif PACKETIZER_LINUX_H
