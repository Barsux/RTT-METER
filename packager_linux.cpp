#include "packager_linux.h"

class PackagerObject: public WaitSystem::Module, public Packager {
    Packager::Setup &setup;
public:
    L2Transport::Queue_rx*     l2_transport_rx;
    L2Transport::Queue_tx*     l2_transport_tx;
    L2Transport::Queue_sent* l2_transport_sent;
    class Rx: public Queue_prx {public:
        PackagerObject &base;
        Rx(PackagerObject &base): base(base){}
    } packager_rx;
    class Tx: public Queue_ptx {public:
        PackagerObject &base;
        Tx(PackagerObject &base): base(base){}
    } packager_tx;
    class Sent: public Queue_psent {public:
        PackagerObject &base;
        Sent(PackagerObject &base): base(base){}
    } packager_sent;

    void attach_l2_transport(L2Transport::Queue_rx* rx, L2Transport::Queue_tx* tx, L2Transport::Queue_sent* sent) {
        disable_wait(l2_transport_rx); disable_wait(l2_transport_tx); disable_wait(l2_transport_sent);
        l2_transport_rx = rx; l2_transport_tx = tx; l2_transport_sent = sent;
        enable_wait(l2_transport_rx);
        enable_wait(l2_transport_sent);
    }


    PackagerObject(WaitSystem* waitSystem, Packager::Setup &setup): WaitSystem::Module(waitSystem)
            ,setup(setup), packager_rx(*this), packager_tx(*this), packager_sent(*this)
    {
        module_debug = "Packager";
        rx = &packager_rx;
        tx = &packager_tx;
        sent = &packager_sent;
        enable_wait(tx);
    }

    void evaluate(){
    }
};
Packager* new_Packager(WaitSystem* waitSystem, Packager::Setup &setup){
    return new PackagerObject(waitSystem, setup);
}
