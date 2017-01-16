#ifndef GEO_NETWORK_CLIENT_MESSAGEREUSLT_H
#define GEO_NETWORK_CLIENT_MESSAGEREUSLT_H

#include "../../../common/NodeUUID.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>

#include <memory>

using namespace std;

typedef boost::posix_time::ptime Timestamp;
typedef boost::posix_time::time_duration Duration;
typedef uint64_t MicrosecondsTimestamp;

class MessageResult {
public:
    typedef shared_ptr<MessageResult> Shared;
    typedef shared_ptr<const MessageResult> SharedConst;

public:
    MessageResult(
        const NodeUUID &senderUUID,
        const uint16_t resultCode
    );

    const NodeUUID &commandUUID() const;

    const uint16_t resultCode() const;

    const Timestamp &timestampCompleted() const;

    const string serialize() const;

private:
    NodeUUID mSenderUUID;
    uint16_t mResultCode;
    Timestamp mTimestampCompleted;

};


#endif //GEO_NETWORK_CLIENT_MESSAGEREUSLT_H
