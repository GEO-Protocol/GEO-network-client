#ifndef GEO_NETWORK_CLIENT_OBSERVINGTRANSACTION_H
#define GEO_NETWORK_CLIENT_OBSERVINGTRANSACTION_H

#include "messages/ObservingClaimAppendRequestMessage.h"
#include "../contractors/addresses/IPv4WithPortAddress.h"

#include "../common/time/TimeUtils.h"

class ObservingTransaction {

public:
    typedef shared_ptr<ObservingTransaction> Shared;

    enum ObservingResponseType {
        NoInfo = 0,
        ClaimInPool = 1,
        ClaimInBlock = 2,
        ParticipantsVotesPresent = 3,

        ClaimTimeExpired = 4,
        RejectedByObserving = 5,
    };
    typedef byte SerializedObservingResponseType;

    ObservingTransaction(
        ObservingClaimAppendRequestMessage::Shared observingRequestMessage,
        IPv4WithPortAddress::Shared requestedObserver,
        ObservingResponseType initialObservingResponse);

    ObservingClaimAppendRequestMessage::Shared observingRequestMessage() const;

    ObservingResponseType observingResponseType() const;

    void setObservingResponseType(
        ObservingResponseType observingResponseType);

    const TransactionUUID& transactionUUID() const;

    /**
     * @returns date time when next action must be scheduled.
     */
    const DateTime &nextActionDateTime();

    void rescheduleNextActionTime();

private:
    static const uint16_t kClaimRequestPeriodSeconds = 60;

private:
    ObservingClaimAppendRequestMessage::Shared mRequest;
    ObservingResponseType mResponse;
    vector<IPv4WithPortAddress::Shared> mRequestedObservers;

    // Stores date time, when action from this queue must be performed.
    DateTime mNextActionDateTime;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGTRANSACTION_H
