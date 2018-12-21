#include "TopologyTrustLine.h"

TopologyTrustLine::TopologyTrustLine(
    ContractorID sourceID,
    ContractorID targetID,
    ConstSharedTrustLineAmount amount):

    mSourceID(sourceID),
    mTargetID(targetID),
    mAmount(amount),
    mUsedAmount(
        make_shared<TrustLineAmount>(0))
{
    if (*amount.get() < TrustLine::kZeroAmount()) {
        throw ValueError("TopologyTrustLine::TopologyTrustLine: "
                             "Amount can't be negative value.");
    }
}

const ContractorID TopologyTrustLine::sourceID() const
{
    return mSourceID;
}

const ContractorID TopologyTrustLine::targetID() const
{
    return mTargetID;
}

ConstSharedTrustLineAmount TopologyTrustLine::amount() const
{
    return mAmount;
}

ConstSharedTrustLineAmount TopologyTrustLine::freeAmount()
{
    return ConstSharedTrustLineAmount(
        new TrustLineAmount(
            *mAmount.get() - *mUsedAmount.get()));
}

void TopologyTrustLine::addUsedAmount(
    const TrustLineAmount &amount)
{
    *mUsedAmount.get() = *mUsedAmount.get() + amount;
}

void TopologyTrustLine::setUsedAmount(
    const TrustLineAmount &amount)
{
    *mUsedAmount.get() = amount;
}

void TopologyTrustLine::setAmount(
    ConstSharedTrustLineAmount amount)
{
    mAmount = amount;
}