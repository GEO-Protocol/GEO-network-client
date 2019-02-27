#include "BlockNumberRecourse.h"

BlockNumberRecourse::BlockNumberRecourse(
    const TransactionUUID& transactionUUID,
    BlockNumber actualObservingBlockNumber):

    BaseResource(
        BaseResource::ObservingBlockNumber,
        transactionUUID),

    mActualObservingBlockNumber(actualObservingBlockNumber)
{}

BlockNumber BlockNumberRecourse::actualObservingBlockNumber() const
{
    return mActualObservingBlockNumber;
}