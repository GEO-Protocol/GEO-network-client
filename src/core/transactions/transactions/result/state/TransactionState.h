#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"

#include "../../../../network/messages/Message.hpp"

#include <stdint.h>
#include <vector>


using namespace std;

class TransactionState {
public:
    typedef shared_ptr<TransactionState> Shared;
    typedef shared_ptr<const TransactionState> SharedConst;
    typedef uint64_t AwakeTimestamp;

public:
    TransactionState(
        MicrosecondsTimestamp awakeTimestamp,
        bool flushToPermanentStorage = false);

    TransactionState(
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    TransactionState(
        MicrosecondsTimestamp awakeTimestamp,
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    ~TransactionState();

public:
    static TransactionState::SharedConst awakeAsFastAsPossible();

    static TransactionState::SharedConst awakeAfterTimeout(
        MicrosecondsTimestamp microseconds);

    const MicrosecondsTimestamp awakeningTimestamp() const;

    const vector<Message::MessageTypeID> &acceptedMessagesTypes() const;

    const bool needSerialize() const;

private:
    MicrosecondsTimestamp mAwakeningTimestamp;
    vector<Message::MessageTypeID> mRequiredMessageTypes;
    bool mFlushToPermanentStorage;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
