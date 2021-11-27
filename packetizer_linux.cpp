#include "packetizer_linux.h"


class PacketizerObject: public WaitSystem::Module, public Packetizer {
    Packetizer::Setup &setup;
public:
    bool setted;
    struct pckt packet;
    bool is_server;
    bool ready; // Пока не пойму как правильней это сделать //
    L2Transport::Queue_rx*     l2_transport_rx;
    L2Transport::Queue_tx*     l2_transport_tx;
    L2Transport::Queue_sent*  l2_transport_sent;

    Global_setup::Queue_toSet*       setup_set;
    Global_setup::Queue_toSave*     setup_save;

    class Rx: public Queue_prx {public:
        PacketizerObject &base;
        Rx(PacketizerObject &base): base(base){}
        int recv(int &seq, U64 &tstmp){
            return base.recv(seq, tstmp);
        }
    } prx;
    class Tx: public Queue_ptx {public:
        PacketizerObject &base;
        Tx(PacketizerObject &base): base(base){}
        int send(int seq){
            return base.send(seq);
        }
    } ptx;
    class Sent: public Queue_psent {public:
        PacketizerObject &base;
        Sent(PacketizerObject &base): base(base){}

    } psent;

    UC checksum(unsigned short* buff, int _16bitword)
    {
        unsigned long sum;
        for(sum=0;_16bitword>0;_16bitword--)
            sum+=htons(*(buff)++);
        sum = ((sum >> 16) + (sum & 0xFFFF));
        sum += (sum>>16);
        return (unsigned short)(~sum);
    }

    void attach_l2_transport(L2Transport::Queue_rx* rx, L2Transport::Queue_tx* tx, L2Transport::Queue_sent* sent) {
        disable_wait(l2_transport_rx); disable_wait(l2_transport_tx); disable_wait(l2_transport_sent);
        l2_transport_rx = rx; l2_transport_tx = tx; l2_transport_sent = sent;
        enable_wait(l2_transport_rx);
        enable_wait(l2_transport_sent);
        enable_wait(l2_transport_tx);
    }

    void attach_Global_setup(Global_setup::Queue_toSave* save, Global_setup::Queue_toSet* set){
        setup_set = set; setup_save = save;
        disable_wait(setup_set); disable_wait(setup_save);
        enable_wait(setup_set);
    }

    PacketizerObject(WaitSystem* waitSystem, Packetizer::Setup &setup): WaitSystem::Module(waitSystem)
            , setted(false) , ready(false), setup(setup), prx(*this), ptx(*this), psent(*this), setup_set(), setup_save()
    {
        module_debug = "PACKETIZER";
        rx = &prx;
        tx = &ptx;
        sent = &psent;
        enable_wait(tx);
        enable_wait(rx);
        flags |= evaluate_every_cycle;
    }

    int recv(){

    }
    int send(int seq){
        int total_len = 0;
        U8 buffer[packet.size];
        bzero(buffer, packet.size);
        //<ETH>
        struct ethheader *eth = (struct ethheader *)(buffer);
        memcpy(eth->h_source, packet.srcMAC, 6);
        memcpy(eth->h_dest, packet.dstMAC, 6);
        eth->h_proto = htons(0x0800);
        total_len += sizeof(struct ethheader);

        struct ipheader *iphdr = (struct ipheader*)(buffer + sizeof(struct ethheader));
        iphdr->ihl = 5;
        iphdr->version = 4;
        iphdr->tos=16;
        iphdr->id = htons(10241);
        iphdr->ttl = 64;
        iphdr->protocol = IPPROTO_UDP;
        iphdr->saddr = packet.srcIP;
        iphdr->daddr = packet.dstIP;
        iphdr->check = 0;
        total_len += sizeof(struct ipheader);

        struct udpheader *udp = (struct udpheader *)(buffer + sizeof(struct ipheader) + sizeof(struct ethheader));
        udp->source = htons(packet.srcPORT);
        udp->dest = htons(packet.dstPORT);
        udp->check = 0;
        total_len += sizeof(struct udpheader);

        struct rttheader *rtt = (struct rttheader *)(buffer + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader));
        rtt->rttproto = 5850;
        rtt->sequence = seq;
        total_len += sizeof(struct rttheader);
        udp->len = htons((packet.size- sizeof(struct ipheader) - sizeof(struct ethheader)));
        iphdr->tot_len = htons(packet.size - sizeof(struct ethheader));
        for (int i = total_len + 1; i < packet.size; i++){
            buffer[i] = i % 10 + 48;
        }

        iovec iovec; iovec.iov_base = buffer; iovec.iov_len = packet.size;
        msghdr msg = {}; msg.msg_iov = &iovec; msg.msg_iovlen = 1;

        short status = l2_transport_tx->send(msg);
        if (status > 0) print("PACKET SENT!");
    }

    int recv(int &seq, U64 &tstmp){
        bool rttpacket = false;
        U8 packet[2048];
        //ЗАГЛУШКА
        seq = 1;
        //ЗАГЛУШКА
        __be16 ip_proto = htons(0x0800);
        iovec iovec; iovec.iov_base = packet; iovec.iov_len = sizeof(packet);
        msghdr msg; msg.msg_iov = &iovec; msg.msg_iovlen = 1;
        sockaddr_ll sa_ll; msg.msg_name = &sa_ll; msg.msg_namelen = sizeof(sa_ll);
        U8 t[256]; msg.msg_control = t; msg.msg_controllen = sizeof(t); msg.msg_flags = 0;
        int r = l2_transport_rx->recv(msg);
        if(r < 0) {
            return -1;
            print("ERROR WITH RECV");
        }
        struct ethheader *eth = (struct ethheader *)(packet);
        char srcMAC[17], dstMAC[17];
        char srcIP[15], dstIP[15];
        mac2str(srcMAC, eth->h_source);
        mac2str(dstMAC, eth->h_dest);
        if(eth->h_proto == ip_proto) {
            struct ipheader *ip = (struct ipheader *) (packet + sizeof(struct ethheader));
            ip42str(srcIP, ip->saddr);
            ip42str(dstIP, ip->daddr);
            if(ip->protocol == IPPROTO_UDP){
                struct udpheader *uh = (struct udpheader *)(packet +sizeof(ethheader) + sizeof(ipheader));
                short srcPORT, dstPORT;
                srcPORT = ntohs(uh->source);
                dstPORT = ntohs(uh->dest);
                struct rttheader *rtt = (struct rttheader *)(packet + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader));
                if(rtt->rttproto == 5850){
                    rttpacket = true;
                    int sequence = rtt->sequence;
                    print("\nSource MAC  - %s\nDest MAC    - %s\nSource IP   - %s\nDest IP     - %s\nSource Port - %d\nDest Port   - %d\nSequence    - %d\n", srcMAC, dstMAC, srcIP, dstIP, srcPORT, dstPORT, sequence);
                }
            }
        }
        if(!rttpacket){
            return -1;
        }
        cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); tstmp = 0;
        while (!tstmp && cmsg) {
            if (cmsg->cmsg_level==SOL_SOCKET && cmsg->cmsg_type==SCM_TIMESTAMPNS) {
                timespec* ts = (timespec*)CMSG_DATA(cmsg);
                tstmp = U64(ts->tv_sec)*1000000000ULL + ts->tv_nsec;
                break;
            }
            cmsg = CMSG_NXTHDR(&msg, cmsg);
            }
        return 1;
        }


    void evaluate(){
        while (WaitSystem::Queue* queue = enum_ready_queues()){
            if(queue == l2_transport_tx && setted){
                tx->setReady();
                disable_wait(l2_transport_tx);
            }
            else if(queue == l2_transport_rx && setted){
                rx->setReady();
            }
            else if(queue == rx && !setted){
                packet = rx->packet;
                print("I get values");
                if(packet.is_server){
                    print("I am server!");
                }
                rx->clear();
                setted = true;
            }
            else if(queue == tx){
                enable_wait(l2_transport_tx);
            }
        }

    }
};
Packetizer* new_Packetizer(WaitSystem* waitSystem, Packetizer::Setup &setup){
    return new PacketizerObject(waitSystem, setup);
}
