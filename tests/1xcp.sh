#!/bin/bash

cd ..
make
cd tests || true
make clean
make -j4
./1xcp || exit
../parse_output xcp.log -ascii | grep -v -i -e cum_Traffic -e overlow -e range > xcp_rate
../parse_output xcp.log -ascii | grep -i -e range > xcp_q && ./1tcp
../parse_output tcp.log -ascii | grep -vie cum_traffic -e overlow -e range > tcp_rate
../parse_output tcp.log -ascii | grep -i -e range > tcp_q
