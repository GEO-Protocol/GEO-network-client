#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H

#include "../../../../common/time/TimeUtils.h"
#include "../../../../network/messages/Message.hpp"
#include "../../../../resources/resources/BaseResource.h"

#include "boost/date_time.hpp"

#include <stdint.h>
#include <vector>


using namespace std;


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
        uint32_t milliseconds);

    static TransactionState::SharedConst waitForMessageTypes(
        vector<Message::MessageTypeID> &&requiredMessageType,
        uint32_t noLongerThanMilliseconds = 0);

    static TransactionState::SharedConst waitForResourcesTypes(
        vector<BaseResource::ResourceType> &&requiredResourcesType,
        uint32_t noLongerThanMilliseconds = 0);

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

    const vector<BaseResource::ResourceType>& acceptedResourcesTypes() const;

    const bool needSerialize() const;

    const bool mustBeRescheduled() const;

    const bool mustExit() const;

private:
    GEOEpochTimestamp mAwakeningTimestamp;
    vector<Message::MessageTypeID> mRequiredMessageTypes;
    vector<BaseResource::ResourceType> mRequiredResourcesTypes;
    bool mFlushToPermanentStorage;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSTATE_H
