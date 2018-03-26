/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "MaxFlowCalculationTrustLine.h"

MaxFlowCalculationTrustLine::MaxFlowCalculationTrustLine(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID,
    ConstSharedTrustLineAmount amount):

    mSourceUUID(sourceUUID),
    mTargetUUID(targetUUID),
    mAmount(amount),
    mUsedAmount(make_shared<TrustLineAmount>(0))
{
    if (*amount.get() < TrustLine::kZeroAmount()) {
        throw ValueError("MaxFlowCalculationTrustLine::MaxFlowCalculationTrustLine: "
                             "Amount can't be negative value.");
    }
}

const NodeUUID& MaxFlowCalculationTrustLine::sourceUUID() const
{
    return mSourceUUID;
}

const NodeUUID& MaxFlowCalculationTrustLine::targetUUID() const
{
    return mTargetUUID;
}

ConstSharedTrustLineAmount MaxFlowCalculationTrustLine::amount() const
{
    return mAmount;
}

ConstSharedTrustLineAmount MaxFlowCalculationTrustLine::freeAmount()
{
    return ConstSharedTrustLineAmount(
        new TrustLineAmount(
            *mAmount.get() - *mUsedAmount.get()));
}

void MaxFlowCalculationTrustLine::addUsedAmount(const TrustLineAmount &amount)
{
    *mUsedAmount.get() = *mUsedAmount.get() + amount;
}

void MaxFlowCalculationTrustLine::setUsedAmount(const TrustLineAmount &amount)
{
    *mUsedAmount.get() = amount;
}

void MaxFlowCalculationTrustLine::setAmount(ConstSharedTrustLineAmount amount)
{
    mAmount = amount;
}