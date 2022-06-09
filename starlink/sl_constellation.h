#include "isl.h"

class SLConstellation {
    public:
        SLConstellation(EventList& eventlist);
    private:
        Link_factory _link_factory;
        int sat_index_to_id(int sat_index);
        int sat_id_to_index(int sat_id);
};