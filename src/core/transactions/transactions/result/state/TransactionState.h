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

public:
    TransactionState(
        Milliseconds timeout,
        bool flushToPermanentStorage = false);

    TransactionState(
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    TransactionState(
        Milliseconds timeout,
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    ~TransactionState();

    static TransactionState::SharedConst exit();

    static TransactionState::SharedConst awakeAsFastAsPossible();

    static TransactionState::SharedConst awakeAfterMilliseconds(
        Milliseconds milliseconds);

    static TransactionState::SharedConst waitForMessageTypes(
        vector<Message::MessageTypeID> &&requiredMessageType,
        Milliseconds noLongerThanMilliseconds=0);

    const MicrosecondsTimestamp awakeningTimestamp() const;

    const vector<Message::MessageTypeID>& acceptedMessagesTypes() const;

    const bool needSerialize() const;

    const bool mustBeRescheduled() const;



private:
    MicrosecondsTimestamp mAwakeningTimestamp;
    vector<Message::MessageTypeID> mRequiredMessageTypes;
    bool mFlushToPermanentStorage;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
