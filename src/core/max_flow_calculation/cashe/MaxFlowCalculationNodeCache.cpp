#include "MaxFlowCalculationNodeCache.h"

MaxFlowCalculationNodeCache::MaxFlowCalculationNodeCache(
    const TrustLineAmount &amount) :
    mCurrentFlow(amount),
    mFinalFlow(false),
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
