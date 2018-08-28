/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TIME_UTILS_H
#define GEO_NETWORK_CLIENT_TIME_UTILS_H

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <cstdint>


using namespace std;
namespace pt = boost::posix_time;
namespace gt = boost::gregorian;
namespace dt = boost::date_time;


typedef pt::ptime DateTime;
typedef pt::time_duration Duration;
typedef int64_t GEOEpochTimestamp;


inline const DateTime GEOEpoch() {

    static pt::ptime GEOEpoch = pt::time_from_string("2016-02-02 00:00:00.000");
    return GEOEpoch;
}

inline DateTime utc_now() {

    return pt::microsec_clock::universal_time();
}

inline GEOEpochTimestamp microsecondsSinceGEOEpoch(
    DateTime datetime) {

    Duration duration = datetime - GEOEpoch();
    return duration.total_microseconds();
}

inline DateTime dateTimeFromGEOEpochTimestamp(
    GEOEpochTimestamp timestamp) {

    auto epoch = GEOEpoch();
    DateTime t(epoch + pt::microseconds(timestamp));
    return t;
}

#endif //GEO_NETWORK_CLIENT_TIME_UTILS_H
