#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../CommandUUID.h"
#include "../../../common/NodeUUID.h"

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <string>


namespace uuids = boost::uuids;
typedef boost::posix_time::ptime Timestamp;


class BaseUserCommand {
public:
    static const constexpr char kCommandsSeparator = '\n';
    static const constexpr char kTokensSeparator = '\t';

public:
    BaseUserCommand(
        const CommandUUID &uuid,
        const string &identifier);

    const string &identifier() const;
    const CommandUUID &uuid() const;
    const Timestamp &timestampAccepted() const;

protected:
    const CommandUUID mUUID;
    const string mIdentifier;
    const Timestamp mTimestampAccepted;

protected:
    virtual void deserialize(
        const string &commandBuffer) = 0;
};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
