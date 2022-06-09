#include "sl_constellation.h"
#include "eventlist.h"
#include "loggers.h"

int main(){
    EventList eventlist;
    Logfile logfile("sl_main.log", eventlist);
    logfile.setStartTime(0);
    Link::_logfile = &logfile;
    Link::_queue_logger_sampling_interval = timeFromMs(10);
    // eventlist.setEndtime(timeFromSec(7200));
    eventlist.setEndtime(timeFromSec(120));

    Logfile sink_logfile("main_starlink_sink.log", eventlist);
    sink_logfile.setStartTime(0);
    TcpSinkLoggerSampling sink_Logger(timeFromMs(500), eventlist);
    sink_logfile.addLogger(sink_Logger);


    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(10), eventlist);


    SLConstellation constellation(eventlist);
    
    // bool in;


    // while (eventlist.doNextEvent()) {
    // }

    // return 0;
}