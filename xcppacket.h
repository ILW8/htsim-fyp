// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-                                              
#ifndef XCPPACKET_H
#define XCPPACKET_H

#include <list>
#include "network.h"



// XcpPacket and XcpAck are subclasses of Packet.
// They incorporate a packet database, to reuse packet objects that are no longer needed.
// Note: you never construct a new XcpPacket or XcpAck directly; 
// rather you use the static method newpkt() which knows to reuse old packets from the database.

class XcpPacket : public Packet {
public:
    typedef uint64_t seq_t;

    inline static XcpPacket* newpkt(PacketFlow &flow, const Route &route, 
				    seq_t seqno, int size,
				    uint32_t cwnd, int32_t demand, simtime_picosec rtt) {
	XcpPacket* p = _packetdb.allocPacket();
	p->set_route(flow,route,size,seqno+size-1); // The TCP sequence number is the first byte of the packet; I will ID the packet by its last byte.
	p->_type = XCP;
	p->_seqno = seqno;
	p->_syn = false;
	p->_rtt = rtt;
	p->_cwnd = cwnd;
	p->_demand = demand;  // Bytes
	return p;
    }

    inline static XcpPacket* new_syn_pkt(PacketFlow &flow, const Route &route, 
					 seq_t seqno, int size, int32_t demand) {
	XcpPacket* p = newpkt(flow,route,seqno, size, size, demand, 0);
	p->_syn = true;
	return p;
    }

    void free() {_packetdb.freePacket(this);}
    virtual ~XcpPacket(){}
    inline seq_t seqno() const {return _seqno;}
    inline simtime_picosec ts() const {return _ts;}
    inline void set_ts(simtime_picosec ts) {_ts = ts;}
    inline simtime_picosec rtt() const {return _rtt;}
    inline uint32_t cwnd() const {return _cwnd;}
    inline int32_t demand() const {return _demand;}
    inline void set_demand(int32_t demand)  {_demand = demand;}
protected:
    seq_t _seqno;
    bool _syn;
    simtime_picosec _ts;
    simtime_picosec _rtt;
    uint32_t _cwnd;
    int32_t _demand;
    static PacketDB<XcpPacket> _packetdb;
};

class XcpAck : public Packet {
public:
    typedef XcpPacket::seq_t seq_t;

    inline static XcpAck* newpkt(PacketFlow &flow, const Route &route, 
				 seq_t seqno, seq_t ackno,
				 uint32_t cwnd_echo, int32_t demand, simtime_picosec rtt_echo) {
	XcpAck* p = _packetdb.allocPacket();
	p->set_route(flow,route,ACKSIZE,ackno);
	p->_type = XCPACK;
	p->_seqno = seqno;
	p->_ackno = ackno;
	p->_cwnd_echo = cwnd_echo;
	p->_allowed_demand = demand;
	p->_rtt_echo = rtt_echo;
	return p;
    }

    void free() {_packetdb.freePacket(this);}
    inline seq_t seqno() const {return _seqno;}
    inline seq_t ackno() const {return _ackno;}
    inline simtime_picosec ts_echo() const {return _ts_echo;}
    inline void set_ts_echo(simtime_picosec ts) {_ts_echo = ts;}
    inline int32_t allowed_demand() const {return _allowed_demand;}

    virtual ~XcpAck(){}
    const static int ACKSIZE=40;
protected:
    seq_t _seqno;
    seq_t _ackno;
    simtime_picosec _ts_echo;
    simtime_picosec _rtt_echo; // echoed so we know what we sent with the data packet
    uint32_t _cwnd_echo;  // echoed so we know what we sent with the data packet
    int32_t _allowed_demand;
    static PacketDB<XcpAck> _packetdb;
};

class XcpCtlPacket : public Packet {
public:
    typedef uint64_t seq_t;

    inline static XcpCtlPacket* newpkt(PacketFlow &flow, const Route &route) {
	XcpCtlPacket* p = _packetdb.allocPacket();
	p->set_route(flow,route,CTLSIZE,(uintptr_t)p); // The TCP sequence number is the first byte of the packet; I will ID the packet by its last byte.
	p->_type = XCPCTL;
	p->_free_throughput = INT64_MAX;  // Bytes
	return p;
    }

    void free() {_packetdb.freePacket(this);}
    virtual ~XcpCtlPacket(){}
    inline simtime_picosec ts() const {return _ts;}
    inline void set_ts(simtime_picosec ts) {_ts = ts;}
    inline linkspeed_bps throughput() const {return _free_throughput;}
    inline void set_throughput(linkspeed_bps throughput)  {_free_throughput = throughput;}

    static const int CTLSIZE = 40;
protected:
    simtime_picosec _ts;
    linkspeed_bps _free_throughput;
    static PacketDB<XcpCtlPacket> _packetdb;
};


class XcpCtlAck : public Packet {
public:
    typedef XcpCtlPacket::seq_t seq_t;

    inline static XcpCtlAck* newpkt(PacketFlow &flow, const Route &route, linkspeed_bps throughput) {
	XcpCtlAck* p = _packetdb.allocPacket();
	p->set_route(flow,route,ACKSIZE,(uintptr_t)p);
	p->_type = XCPCTLACK;
	p->_allowed_throughput = throughput;
	return p;
    }

    void free() {_packetdb.freePacket(this);}
    inline simtime_picosec ts_echo() const {return _ts_echo;}
    inline void set_ts_echo(simtime_picosec ts) {_ts_echo = ts;}
    inline linkspeed_bps allowed_throughput() const {return _allowed_throughput;}

    virtual ~XcpCtlAck(){}
    const static int ACKSIZE=40;
protected:
    simtime_picosec _ts_echo;
    linkspeed_bps _allowed_throughput;
    static PacketDB<XcpCtlAck> _packetdb;
};

#endif
