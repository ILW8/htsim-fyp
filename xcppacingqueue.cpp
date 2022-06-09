
#include "xcppacingqueue.h"
#include <math.h>
#include <iostream>
#include "xcppacket.h"
#include "xcp.h"

XcpPacingQueue::XcpPacingQueue(EventList& eventlist)
        : Queue(99999999,99999999,eventlist,NULL),  // probably bad to hardcode bitrate/maxsz
        _latestRTT(0),
        _nextPacketReleaseTime(0)
{
    cout << "Using XCP pacing queue" << endl;
}

simtime_picosec XcpPacingQueue::getInterPacketSpacing(XcpPacket* pkt){
    return pkt->size() * _latestRTT / pkt->cwnd() / 2;
}

void XcpPacingQueue::completeService() {
    if (eventlist().now() < _nextPacketReleaseTime){  // I forget why I put this here, but I'm now too scared to delete
        return;
    }
    if (_enqueued.empty()) {
        // queue must have been explicitly cleared
        return;
    }

    /* dequeue the packet */
    Packet* pkt = _enqueued.back();
    _enqueued.pop_back();
    _queuesize -= pkt->size();

    /* tell the packet to move on to the next pipe */
    pkt->sendOn();

    if (!_enqueued.empty()) {
        /* schedule the next dequeue event */
        XcpPacket *xpkt = dynamic_cast<XcpPacket*>(_enqueued.back());
        _nextPacketReleaseTime = eventlist().now() + getInterPacketSpacing(xpkt);
        eventlist().sourceIsPending(*this, _nextPacketReleaseTime);
    }
}

void XcpPacingQueue::receivePacket(Packet &pkt) {
    if (pkt.type() != XCP){
        return;

    }
    XcpPacket *xpkt = dynamic_cast<XcpPacket*>(&pkt);
    _latestRTT = xpkt->rtt();
    // _lastPacketTime = eventlist().now();

    /* enqueue the packet */
    bool queueWasEmpty = _enqueued.empty();
    _enqueued.push_front(&pkt);

    if (queueWasEmpty) {
        /* schedule the dequeue event */
        assert(_enqueued.size() == 1);
        /* schedule the next dequeue event (this used to be beginService()) */
        if (_enqueued.empty()) {
            // queue must have been explicitly cleared
            return;
        }

        _nextPacketReleaseTime = eventlist().now() + getInterPacketSpacing(xpkt);
        eventlist().sourceIsPending(*this, _nextPacketReleaseTime);
    }
}