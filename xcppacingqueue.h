
#include "queue.h"
#include <map>
#include <list>
#include "config.h"
#include "eventlist.h"
#include "network.h"
#include "xcp.h"

#ifndef STARLINK_HTSIM_XCPPACINGQUEUE_H
#define STARLINK_HTSIM_XCPPACINGQUEUE_H


class XcpPacingQueue : public Queue {
public:
    XcpPacingQueue(EventList &eventlist);
    virtual void receivePacket(Packet &pkt);
    virtual void completeService();
private:
    simtime_picosec _latestRTT;
    simtime_picosec _nextPacketReleaseTime;
    virtual simtime_picosec getInterPacketSpacing(XcpPacket* pkt);
};


#endif //STARLINK_HTSIM_XCPPACINGQUEUE_H
