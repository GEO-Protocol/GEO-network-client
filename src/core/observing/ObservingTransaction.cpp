#include "ObservingTransaction.h"

ObservingTransaction::ObservingTransaction(
    ObservingClaimAppendRequestMessage::Shared observingRequestMessage) :
    mRequest(observingRequestMessage),
    mResponse(NoInfo)
{
    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestPeriodSeconds);
#ifdef TESTS
    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestPeriodSecondsTests);
#endif
}

void ObservingTransaction::addRequestedObserver(
    IPv4WithPortAddress::Shared requestedObserver)
{
    mRequestedObservers.push_back(
        requestedObserver);
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

IPv4WithPortAddress::Shared ObservingTransaction::requestedObserver() const
{
    return mRequestedObservers.front();
}

const DateTime &ObservingTransaction::nextActionDateTime()
{
    return mNextActionDateTime;
}

void ObservingTransaction::rescheduleNextActionTime()
{
    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestPeriodSeconds);
#ifdef TESTS
    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestPeriodSecondsTests);
#endif
}

void ObservingTransaction::rescheduleNextActionSmallTime()
{
   mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestSmallPeriodSeconds);
#ifdef TESTS
    mNextActionDateTime = utc_now() + boost::posix_time::seconds(kClaimRequestSmallPeriodSecondsTests);
#endif
}