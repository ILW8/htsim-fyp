#include "sl_satellite.h"
#include <math.h>

SLSatellite::SLSatellite(EventList& eventlist, int sat_id, int plane_position, double sat_alt, double inclination_deg, int orbital_plane) : 
EventSource(eventlist, "SLSat"), sat_id(sat_id), sat_altitude(sat_alt), inclination_deg(inclination_deg), orbital_plane(orbital_plane), plane_pos(plane_position) {
    // sat around 1150.0Km up, but we'll leave it as a variable
    _rotation_matrix = _get_rotation_matrix(inclination_deg, 0, 360.0 * orbital_plane/ORBITAL_PLANE_NUM);
    // cout << _rotation_matrix << endl;
    _origin_theta = plane_pos * 360.0/SATS_PER_PLANE;
    eventlist.sourceIsPendingRel(*this, 0);
}

void SLSatellite::update_coordinates(simtime_picosec time){
    double orbital_period = 2 * PI * (EARTH_RADIUS + sat_altitude)/(sqrt(398600.5/(EARTH_RADIUS + sat_altitude))); // in seconds
    double true_anomaly = fmod(360.0 * time / timeFromSec(orbital_period), 360.0);  // technically true longitude? in degrees
    // cout << printf("Sat%d %f", sat_id, true_anomaly) << endl;
    Eigen::Vector3d base_coords;  // in cartesian coord space
    base_coords << (EARTH_RADIUS + sat_altitude) * cos((true_anomaly + _origin_theta) * PI/180.0), 
                   (EARTH_RADIUS + sat_altitude) * sin((true_anomaly + _origin_theta) * PI/180.0),
                   0;
    Eigen::Vector3d transformed_coords = _rotation_matrix * base_coords;   // in ECEF
    coords_x = transformed_coords[0];
    coords_y = transformed_coords[1];
    coords_z = transformed_coords[2];
    cout << printf("Sat%d %f %f %f", sat_id, coords_x, coords_y, coords_z) << endl;
}

void SLSatellite::doNextEvent(){
    // cout << sat_id << " queuing next event" << endl;
    eventlist().sourceIsPendingRel(*this, SAT_POS_UPDATE_PERIOD_MS);
    update_coordinates(eventlist().now());
}

Eigen::Matrix3d SLSatellite::_get_rotation_matrix(double x_rot_deg, double y_rot_deg, double z_rot_degree){
    Eigen::Matrix3d x_rot;
    x_rot << 1,                          0,                           0,
             0,                          cos(x_rot_deg * PI/180.0),   -sin(x_rot_deg * PI/180.0),
             0,                          sin(x_rot_deg * PI/180.0),   cos(x_rot_deg * PI/180.0);
    Eigen::Matrix3d y_rot;
    y_rot << cos(y_rot_deg * PI/180.0),   0,                          -sin(y_rot_deg * PI/180.0),
             0                        ,   1,                          0,
             sin(y_rot_deg * PI/180.0),   0,                          cos(y_rot_deg * PI/180.0);
    Eigen::Matrix3d z_rot;    
    z_rot << cos(z_rot_degree * PI/180.0),   -sin(z_rot_degree * PI/180.0), 0,
             sin(z_rot_degree * PI/180.0),   cos(z_rot_degree * PI/180.0),  0,
             0,                           0,                          1;
    
    return z_rot * y_rot * x_rot;
}