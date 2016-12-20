#ifndef GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H
#define GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H


#include "../commands/commands/BaseUserCommand.h"

#include "boost/date_time/posix_time/ptime.hpp"


class BaseCommandResult {
public:
    BaseCommandResult(
        const uint16_t &resultCode);

    const uint16_t resultCode() const;
    const Timestamp &timestampCompleted() const;

    virtual string serialize() = 0;

private:
    uint16_t mResultCode;
    Timestamp mTimestampCompleted;
};


#endif //GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H
