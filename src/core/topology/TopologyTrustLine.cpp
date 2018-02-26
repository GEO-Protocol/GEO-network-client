#include "TopologyTrustLine.h"

TopologyTrustLine::TopologyTrustLine(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID,
    ConstSharedTrustLineAmount amount):

    mSourceUUID(sourceUUID),
    mTargetUUID(targetUUID),
    mAmount(amount),
    mUsedAmount(make_shared<TrustLineAmount>(0))
{
    if (*amount.get() < TrustLine::kZeroAmount()) {
        throw ValueError("TopologyTrustLine::TopologyTrustLine: "
                             "Amount can't be negative value.");
    }
}

const NodeUUID& TopologyTrustLine::sourceUUID() const
{
    return mSourceUUID;
}

const NodeUUID& TopologyTrustLine::targetUUID() const
{
    return mTargetUUID;
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

void TopologyTrustLine::addUsedAmount(const TrustLineAmount &amount)
{
    *mUsedAmount.get() = *mUsedAmount.get() + amount;
}

void TopologyTrustLine::setUsedAmount(const TrustLineAmount &amount)
{
    *mUsedAmount.get() = amount;
}

void TopologyTrustLine::setAmount(ConstSharedTrustLineAmount amount)
{
    mAmount = amount;
}