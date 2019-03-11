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
        ObservingClaimAppendRequestMessage::Shared observingRequestMessage);

    void addRequestedObserver(
        IPv4WithPortAddress::Shared requestedObserver);

    ObservingClaimAppendRequestMessage::Shared observingRequestMessage() const;

    ObservingResponseType observingResponseType() const;

    void setObservingResponseType(
        ObservingResponseType observingResponseType);

    const TransactionUUID& transactionUUID() const;

    IPv4WithPortAddress::Shared requestedObserver() const;

    /**
     * @returns date time when next action must be scheduled.
     */
    const DateTime &nextActionDateTime();

    void rescheduleNextActionTime();

    void rescheduleNextActionSmallTime();

private:
    static const uint16_t kClaimRequestPeriodSeconds = 60;
    static const uint16_t kClaimRequestSmallPeriodSeconds = 30;

#ifdef TESTS
    static const uint16_t kClaimRequestPeriodSecondsTests = 20;
    static const uint16_t kClaimRequestSmallPeriodSecondsTests = 10;
#endif

private:
    ObservingClaimAppendRequestMessage::Shared mRequest;
    ObservingResponseType mResponse;
    vector<IPv4WithPortAddress::Shared> mRequestedObservers;

    // Stores date time, when action from this queue must be performed.
    DateTime mNextActionDateTime;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGTRANSACTION_H
