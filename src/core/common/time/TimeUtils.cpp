#include "TimeUtils.h"

const Timestamp TimeUtils::epoch() {

    static const Timestamp Epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    return Epoch;
}

const Timestamp TimeUtils::geoEpoch() {

    static boost::posix_time::ptime GEOEpoch(boost::gregorian::date(2015, boost::gregorian::Feb, 2));
    return GEOEpoch;
}

MicrosecondsTimestamp TimeUtils::microsecondsTimestamp(
    Timestamp posixTimestamp) {

    Duration duration = posixTimestamp - epoch();
    return (MicrosecondsTimestamp) duration.total_microseconds();
}

Timestamp TimeUtils::posixTimestamp(
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