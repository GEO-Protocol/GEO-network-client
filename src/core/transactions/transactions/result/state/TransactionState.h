#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H

#include "../../../../network/messages/Message.hpp"

#include "boost/date_time.hpp"

#include <stdint.h>
#include <vector>


using namespace std;
namespace datetime = boost::posix_time;


class TransactionState {
public:
    typedef shared_ptr<TransactionState> Shared;
    typedef shared_ptr<const TransactionState> SharedConst;
    typedef uint64_t AwakeTimestamp;

public:
    // Readble shortcats for states creation.

    // todo: think to move this into seprate file,
    // todo: that would be available for the rest source files.
    static datetime::ptime& GEOEpoch();

    static TransactionState::Shared awakeAsFastAsPossible();
    static TransactionState::Shared awakeAfterMilliseconds(
        uint16_t milliseconds);

public:
    TransactionState(
        uint64_t awakeTimestamp,
        bool flushToPermanentStorage = false);

    TransactionState(
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    TransactionState(
        uint64_t awakeTimestamp,
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    ~TransactionState();

public:
    const uint64_t awakeningTimestamp() const;

    const vector<Message::MessageTypeID>& acceptedMessagesTypes() const;

    const bool needSerialize() const;

private:
    static AwakeTimestamp timestampFromTheGEOEpoch(
        datetime::ptime &timestamp);

private:
    uint64_t mAwakeningTimestamp;
    vector<Message::MessageTypeID> mRequiredMessageTypes;
    bool mFlushToPermanentStorage;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
