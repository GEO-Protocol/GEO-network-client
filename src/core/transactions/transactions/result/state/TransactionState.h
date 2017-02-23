#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H

#include "../../../../common/time/TimeUtils.h"
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

public:
    // Readable shortcuts for states creation.
    static TransactionState::SharedConst exit();
    static TransactionState::SharedConst flushAndContinue();
    static TransactionState::SharedConst awakeAsFastAsPossible();
    static TransactionState::SharedConst awakeAfterMilliseconds(
        uint16_t milliseconds);

    static TransactionState::SharedConst waitForMessageTypes(
        vector<Message::MessageTypeID> &&requiredMessageType,
        uint16_t noLongerThanMilliseconds=0);



public:
    TransactionState(
        GEOEpochTimestamp awakeningTimestamp,
        bool flushToPermanentStorage = false);

    TransactionState(
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);

    TransactionState(
        GEOEpochTimestamp awakeTimestamp,
        Message::MessageTypeID requiredMessageType,
        bool flushToPermanentStorage = false);



    const GEOEpochTimestamp awakeningTimestamp() const;

    const vector<Message::MessageTypeID>& acceptedMessagesTypes() const;

    const bool needSerialize() const;

    const bool mustBeRescheduled() const;

    const bool mustExit() const;

private:
    GEOEpochTimestamp mAwakeningTimestamp;
    vector<Message::MessageTypeID> mRequiredMessageTypes;
    bool mFlushToPermanentStorage;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
