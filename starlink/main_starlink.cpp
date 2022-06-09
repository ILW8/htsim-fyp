// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-                                                                                                               
#include "constellation.h"
#include "ping.h"
#include "tcp.h"
#include "loggers.h"
//#include "isl.h"
//#include "satellite.h"
//#include "city.h"

#define RTT1 timeFromMs(20)
#define RTT2 timeFromMs(20)

int main() {
    EventList eventlist;
    eventlist.setEndtime(timeFromSec(180));
    Logfile logfile("main_starlink.log", eventlist);
    logfile.setStartTime(0);
    Link::_logfile = &logfile;
    Link::_queue_logger_sampling_interval = timeFromMs(200);

    // Logfile sink_logfile("main_starlink_sink.log", eventlist);
    // sink_logfile.setStartTime(0);
    // TcpSinkLoggerSampling sink_Logger(timeFromMs(500), eventlist);
    // sink_logfile.addLogger(sink_Logger);


    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(10), eventlist);

    Packet::set_packet_size(1500);
    Constellation constellation(eventlist,
				speedFromMbps((uint64_t)10000), memFromPkt(100),  // Uplinks
				speedFromMbps((uint64_t)10000), memFromPkt(100),  // Downlinks
				speedFromMbps((uint64_t)10000), memFromPkt(100)); // LISLs

    // TcpSrc* tcp_src;
    // TcpSink* tcp_sink;

    // tcp_src = new TcpSrc(NULL, NULL, eventlist);
    // tcp_src->setName("TcpSrc");
    // tcp_src->_ssthresh = timeAsSec(RTT1+RTT2) * speedFromMbps((uint64_t)10);
    // logfile.writeName(*tcp_src);

    // tcp_sink = new TcpSink();
    // tcp_sink->setName("TcpSink");
    // logfile.writeName(*tcp_sink);


    // tcpRtxScanner.registerTcp(*tcp_src);

    //cout << "create London\n";
    City london(51.5, 0.0, constellation);
    //cout << "create NY\n";
    City newyork(40.76, 73.98, constellation);
    // City newyork(53.3, -6.45, constellation);  // dublin ireland google data center
    //cout << "London: add_uplinks\n";
    //london.add_uplinks(sats, num_sats, 0);
    //cout << "NY: add_uplinks\n";
    //newyork.add_uplinks(sats, num_sats, 0);
    //cout << "London: update_uplinks\n";
    london.update_uplinks(0);
    //cout << "NY: update_uplinks\n";
    newyork.update_uplinks(0);
    //cout << "NY: update_uplinks done\n";
    
    /*
    cout << "London: running\n";
    for (int time = 0; time < 4000; time++) {
	newyork.update_uplinks(timeFromSec(time));
    }
    */
    // CbrSrc src(eventlist, 12000, 100, 0);
    // CbrSink sink;
    //cout << "Find route, Lon-NY\n";
    Route* rt_out = london.find_route(newyork, 0);
    //cout << "Find route, NY-Lon\n";
    Route* rt_back = newyork.find_route(london, 0);
    
    const Node*  rt_out_lastnode;
    const Node* rt_back_lastnode;
    //cout << "Find route, NY-Lon done\n";

    // ##### new comment block
    PingClient pingclient(eventlist, timeFromMs(100), 1500 /*bytes*/);
    PingServer pingserver;
    rt_out->push_back(static_cast<PacketSink*>(&pingserver));
    rt_back->push_back(static_cast<PacketSink*>(&pingclient));

    // rt_out->push_back(static_cast<PacketSink*>(tcp_sink));
    // rt_back->push_back(static_cast<PacketSink*>(tcp_src));


    /*
    cout << "Out:" << endl;
    print_route(*rt_out);
    cout << "\nBack:" << endl;
    print_route(*rt_back);
    */

    // ##### new comment block
    pingclient.connect(*rt_out, *rt_back, pingserver, eventlist.now(), 3600);

    // tcp_src->connect(*rt_out, *rt_back, *tcp_sink, eventlist.now());
    // sink_Logger.monitorSink(tcp_sink);

    /*
    for (int i = 0; i < rt->size(); i++) {
	cout << "hop: " << i << endl;
        PacketSink* sink = rt->at(i);
        cout << sink << endl;
        cout << sink->nodename() << endl;
    }
    */
    print_route(*rt_out);

    simtime_picosec last_route_update = eventlist.now();
    while (eventlist.doNextEvent()) {
        // cout << "Time now: " << timeAsMs(eventlist.now()) << endl;
        simtime_picosec now = eventlist.now();
        if (timeAsMs(now - last_route_update) > 500) {
            rt_out_lastnode  = &static_cast<Link*>( rt_out->at( rt_out->size() - 2))->src();  // size() - 1: link between city and sink
            rt_back_lastnode = &static_cast<Link*>(rt_back->at(rt_back->size() - 2))->src();  // and city never changes, so we're 
                                                                                              // interested in src of size() - 2 
                                                                                              // which is between last sat and city

            // london.update_uplinks(eventlist.now());
            // newyork.update_uplinks(eventlist.now());


            rt_out->decr_refcount();
            rt_back->decr_refcount();

            rt_out = london.find_route(newyork, eventlist.now());
            rt_back = newyork.find_route(london, eventlist.now());

            // ##### new comment block
            rt_out->push_back(static_cast<PacketSink*>(&pingserver));
            rt_back->push_back(static_cast<PacketSink*>(&pingclient));
            pingclient.update_route(*rt_out, *rt_back);

            // rt_out->push_back(static_cast<PacketSink*>(tcp_sink));
            // rt_back->push_back(static_cast<PacketSink*>(tcp_src));
            // tcp_src->replace_route(rt_out);
            // tcp_sink->replace_route(rt_back);
            //
            // if (&static_cast<Link*>( rt_out->at( rt_out->size() - 2))->src() != rt_out_lastnode) {
            //     tcp_sink->sl_drop_inflight_packets(now);
            // }

            last_route_update =	now;
        }
    }
    cout << "Sim done, time now: " << timeAsMs(eventlist.now()) << endl;
}
