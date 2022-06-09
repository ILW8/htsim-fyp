// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-                                                                                                       #ifndef CBR_H
#include "cbr.h"
#include "math.h"
#include <iostream>
#include "cbrpacket.h"
////////////////////////////////////////////////////////////////
//  CBR SOURCE
////////////////////////////////////////////////////////////////

CbrSrc::CbrSrc(EventList &eventlist,linkspeed_bps rate,simtime_picosec active,simtime_picosec idle)
  : EventSource(eventlist,"cbr"),  _bitrate(rate),_crt_id(1),_mss(1000),_flow(NULL)
{
    _period = (simtime_picosec)((pow(10.0,12.0) * 8 * _mss) / _bitrate);
    _sink = NULL;
    _route = NULL;
    _start_active = 0;
    _end_active = 0;
    _idle_time = idle;
    _active_time = active;
    _is_active = false;
}

void 
CbrSrc::connect(route_t& routeout, CbrSink& sink, simtime_picosec starttime) 
{
    _route = &routeout;
    _sink = &sink;
    _flow.id = id; // identify the packet flow with the CBR source that generated it
    _is_active = true;
    _start_active = starttime;
    _end_active = _start_active + (simtime_picosec)(2.0*drand()*_active_time);
    eventlist().sourceIsPending(*this,starttime);
}

void 
CbrSrc::doNextEvent() {
    if (_idle_time==0||_active_time==0){
	send_packet();
	return;
    }

    if (_is_active){
	if (eventlist().now()>=_end_active){
	    _is_active = false;
	    eventlist().sourceIsPendingRel(*this,(simtime_picosec)(2*drand()*_idle_time));
	}
	else
	    send_packet();
    }
    else {
	_is_active = true;
	_start_active = eventlist().now();
	_end_active = _start_active + (simtime_picosec)(2.0*drand()*_active_time);
	send_packet();
    }
}

void 
CbrSrc::send_packet() {
    Packet* p = CbrPacket::newpkt(_flow, *_route, _crt_id++, _mss);
    p->sendOn();

    //  simtime_picosec how_long = _period;
    //simtime_picosec _active_already = eventlist().now()-_start_active;

    //if (_active_time!=0&&_idle_time!=0&&period>)

    eventlist().sourceIsPendingRel(*this,_period);
}

////////////////////////////////////////////////////////////////
//  Cbr SINK
////////////////////////////////////////////////////////////////

CbrSink::CbrSink() 
    : Logged("cbr")  {
    _received = 0;
    _last_id = 0;
    _cumulative_ack = 0;
    _nodename = "cbrsink";
}

// Note: _cumulative_ack is the last byte we've ACKed.
// seqno is the first byte of the new packet.
void
CbrSink::receivePacket(Packet& pkt) {
    _received++;
    _cumulative_ack = _received * 1000;
    _last_id = pkt.id();
    pkt.free();
}	

