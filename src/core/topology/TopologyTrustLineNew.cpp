#include "TopologyTrustLineNew.h"

TopologyTrustLineNew::TopologyTrustLineNew(
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
        throw ValueError("TopologyTrustLine::TopologyTrustLineNew: "
                             "Amount can't be negative value.");
    }
}

const ContractorID TopologyTrustLineNew::sourceID() const
{
    return mSourceID;
}

const ContractorID TopologyTrustLineNew::targetID() const
{
    return mTargetID;
}

ConstSharedTrustLineAmount TopologyTrustLineNew::amount() const
{
    return mAmount;
}

ConstSharedTrustLineAmount TopologyTrustLineNew::freeAmount()
{
    return ConstSharedTrustLineAmount(
        new TrustLineAmount(
            *mAmount.get() - *mUsedAmount.get()));
}

void TopologyTrustLineNew::addUsedAmount(
    const TrustLineAmount &amount)
{
    *mUsedAmount.get() = *mUsedAmount.get() + amount;
}

void TopologyTrustLineNew::setUsedAmount(
    const TrustLineAmount &amount)
{
    *mUsedAmount.get() = amount;
}

void TopologyTrustLineNew::setAmount(
    ConstSharedTrustLineAmount amount)
{
    mAmount = amount;
}