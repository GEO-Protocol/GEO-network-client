/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "MaxFlowCalculationNodeCache.h"

MaxFlowCalculationNodeCache::MaxFlowCalculationNodeCache(
    const TrustLineAmount &amount,
    bool isFinal) :
    mCurrentFlow(amount),
    mFinalFlow(isFinal),
    mTimeLastModified(utc_now())
{}

TrustLineAmount& MaxFlowCalculationNodeCache::currentFlow()
{
    return mCurrentFlow;
}

bool MaxFlowCalculationNodeCache::isFlowFinal()
{
    return mFinalFlow;
}

DateTime MaxFlowCalculationNodeCache::lastModified()
{
    return mTimeLastModified;
}

void MaxFlowCalculationNodeCache::updateCurrentFlow(
    const TrustLineAmount &amount,
    bool isFinal)
{
    mCurrentFlow = amount;
    mFinalFlow = isFinal;
    mTimeLastModified = utc_now();
}
