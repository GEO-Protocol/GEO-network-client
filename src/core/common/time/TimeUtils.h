#ifndef GEO_NETWORK_CLIENT_TIME_UTILS_H
#define GEO_NETWORK_CLIENT_TIME_UTILS_H

#include "../Types.h"

#include <cstdint>

using namespace std;

inline const Timestamp epoch() {

    static const Timestamp Epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    return Epoch;
}

inline const Timestamp geoEpoch(){

    static boost::posix_time::ptime GEOEpoch(boost::gregorian::date(2015, boost::gregorian::Feb, 2));
    return GEOEpoch;
}

inline MicrosecondsTimestamp microsecondsTimestamp(
    Timestamp posixTimestamp) {

    Duration duration = posixTimestamp - epoch();
    return (MicrosecondsTimestamp) duration.total_microseconds();
}

inline Timestamp posixTimestamp(
    MicrosecondsTimestamp microsecondsTimestamp) {

    uint32_t max_uint32_t = std::numeric_limits<uint32_t>::max();
    Timestamp accumulator = epoch();
    while (microsecondsTimestamp > max_uint32_t) {
        accumulator += posix::seconds(max_uint32_t);
        microsecondsTimestamp -= max_uint32_t;
    }
    accumulator += posix::microseconds(microsecondsTimestamp);
    return accumulator;
}


#endif //GEO_NETWORK_CLIENT_TIME_UTILS_H
