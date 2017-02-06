#ifndef GEO_NETWORK_CLIENT_TIME_UTILS_H
#define GEO_NETWORK_CLIENT_TIME_UTILS_H

#include "../Types.h"

#include <cstdint>

using namespace std;

class TimeUtils {

public:
    static const Timestamp epoch();

    static const Timestamp geoEpoch();

    static MicrosecondsTimestamp microsecondsTimestamp(
        Timestamp posixTimestamp);

    static Timestamp posixTimestamp(
        MicrosecondsTimestamp microsecondsTimestamp);
};


#endif //GEO_NETWORK_CLIENT_TIME_UTILS_H
