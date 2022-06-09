#include "sl_constellation.h"
#include "sl_satellite.h"
#include <iostream>

SLConstellation::SLConstellation(EventList& eventlist) : _link_factory(eventlist){
    Node::link_factory = &_link_factory;
    SLSatellite *satellites[(int)ORBITAL_PLANE_NUM * SATS_PER_PLANE];

    // create constellation
    int sat_id = 0;
    for (int plane = 0; plane < ORBITAL_PLANE_NUM; plane++){
        for (int plane_pos = 0; plane_pos < SATS_PER_PLANE; plane_pos++){
            satellites[sat_id] = new SLSatellite(eventlist, sat_id, plane_pos, 1150.0, 53.0, plane);  
            sat_id++;
        }
    }

    linkspeed_bps link_speed = speedFromMbps((uint64_t)10000);

    //  add links between every other satellite in the same plane -- Intra-OP links  ------ ignore this for now
    sat_id = 0;  // sat_id of first satellite in plane
    for (int plane = 0; plane < ORBITAL_PLANE_NUM; plane++){
        cout << "plane: " << plane << " sat_id: " << sat_id << endl;
        // int firstSatInPlane = sat_id;
        // int previousSatInPlane = firstSatInPlane;
        // int nextPlanePos = 2;
        // for (int planeConnectionIndex = 0; planeConnectionIndex < SATS_PER_PLANE; planeConnectionIndex++){
        //     satellites[previousSatInPlane]->add_link_to_dst(*satellites[sat_id + nextPlanePos], link_speed, memFromPkt(10));
        //     cout << "connecting sats " << sat_index_to_id(previousSatInPlane) << " and " << sat_index_to_id(sat_id + nextPlanePos) << endl;
        //     previousSatInPlane++;
        //     nextPlanePos = (nextPlanePos + 1) % SATS_PER_PLANE;
        // }
        int firstSatInPlane = sat_id;
        int previousSatInPlane = firstSatInPlane;
        int nextPlanePos = 1;
        for (int planeConnectionIdx = 0; planeConnectionIdx < SATS_PER_PLANE; planeConnectionIdx++){
            satellites[previousSatInPlane]->add_link_to_dst(*satellites[sat_id + nextPlanePos], link_speed, memFromPkt(10));
            // cout << "connecting sats " << sat_index_to_id(previousSatInPlane) << " and " << sat_index_to_id(sat_id + nextPlanePos) << endl;
            previousSatInPlane++;
            nextPlanePos = (nextPlanePos + 1) % SATS_PER_PLANE;
        }
        sat_id += SATS_PER_PLANE;
    }
    // cout << "sat_id: " << sat_id << endl;

}

int SLConstellation::sat_index_to_id(int sat_index){
    int orbital_plane_id = 1 + sat_index / SATS_PER_PLANE;
    int intra_plane_id = 1 + sat_index % SATS_PER_PLANE;
    return orbital_plane_id * 100 + intra_plane_id;  // xxyy where xx represents orbital plane and yy represents satellite id inside a plane
}

int SLConstellation::sat_id_to_index(int sat_id){
    int orbital_plane_id = (sat_id / 100) - 1;
    int intra_plane_id = (sat_id % 100) - 1;
    return orbital_plane_id * SATS_PER_PLANE + intra_plane_id;
}