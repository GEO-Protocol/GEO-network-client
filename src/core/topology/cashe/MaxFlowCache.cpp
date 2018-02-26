#include "MaxFlowCache.h"

MaxFlowCache::MaxFlowCache(
    const TrustLineAmount &amount,
    bool isFinal) :
    mCurrentFlow(amount),
    mFinalFlow(isFinal),
    mTimeLastModified(utc_now())
{}

TrustLineAmount& MaxFlowCache::currentFlow()
{
    return mCurrentFlow;
}

bool MaxFlowCache::isFlowFinal()
{
    return mFinalFlow;
}

DateTime MaxFlowCache::lastModified()
{
    return mTimeLastModified;
}

void MaxFlowCache::updateCurrentFlow(
    const TrustLineAmount &amount,
    bool isFinal)
{
    mCurrentFlow = amount;
    mFinalFlow = isFinal;
    mTimeLastModified = utc_now();
}
