#include "ObservingTransaction.h"

ObservingTransaction::ObservingTransaction(
    ObservingClaimAppendRequestMessage::Shared observingRequestMessage,
    IPv4WithPortAddress::Shared requestedObserver,
    ObservingResponseType initialObservingResponse) :
    mRequest(observingRequestMessage),
    mResponse(initialObservingResponse)
{
    mRequestedObservers.push_back(
        requestedObserver);

    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestPeriodSeconds);
}

ObservingClaimAppendRequestMessage::Shared ObservingTransaction::observingRequestMessage() const
{
    return mRequest;
}

ObservingTransaction::ObservingResponseType ObservingTransaction::observingResponseType() const
{
    return mResponse;
}

void ObservingTransaction::setObservingResponseType(
    ObservingResponseType observingResponseType)
{
    mResponse = observingResponseType;
}

const TransactionUUID & ObservingTransaction::transactionUUID() const
{
    return mRequest->transactionUUID();
}

const DateTime &ObservingTransaction::nextActionDateTime()
{
    return mNextActionDateTime;
}

void ObservingTransaction::rescheduleNextActionTime()
{
    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestPeriodSeconds);
}