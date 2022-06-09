#include "node.h"

#define PI 3.14159265
#define ORBITAL_PLANE_NUM 72.0
#define SATS_PER_PLANE 22
#define SAT_POS_UPDATE_PERIOD_MS timeFromMs(1000)  // trigger coordinate update every 50ms
// inclination: 53 deg

class SLSatellite : public Node, public EventSource{
    public:
        // SLSatellite(int sat_id, double coords_x, double coords_y, double coords_z, double inclination_deg, int orbital_plane);
        SLSatellite(EventList& eventlist, int sat_id, int plane_position, double sat_alt, double inclination_deg, int orbital_plane);

        void doNextEvent();

        int sat_id;
        double coords_z;
        double coords_x;
        double coords_y;
        double sat_altitude;
        double inclination_deg;
        int orbital_plane;
        int plane_pos;

        void update_coordinates(simtime_picosec time);
    private:
        simtime_picosec _last_coords_update_time;
        Eigen::Matrix3d _get_rotation_matrix(double x_rot_deg, double y_rot_deg, double z_rot_degree);
        Eigen::Matrix3d _rotation_matrix;
        double _origin_theta;  // when considering orbit without inclination and orbital plane, where does this satellite fit in?

};