#ifndef GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H
#define GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H

#include <string>

#include "../commands/CommandUUID.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>

typedef boost::posix_time::ptime Timestamp;
typedef boost::posix_time::time_duration Duration;
typedef uint64_t microseconds_timestamp;

class CommandResult {
private:
    CommandUUID mCommandUUID;
    uint16_t mResultCode;
    Timestamp mTimestampCompleted;
    string mResultInformation;

public:
    CommandResult(
            const CommandUUID &commandUUID,
            const uint16_t resultCode);

    CommandResult(
            const CommandUUID &commandUUID,
            const uint16_t resultCode,
            string &resultInformation);

    const CommandUUID &commandUUID() const;

    const uint16_t resultCode() const;

    const Timestamp &timestampCompleted() const;

    const string serialize() const;

};


#endif //GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H
