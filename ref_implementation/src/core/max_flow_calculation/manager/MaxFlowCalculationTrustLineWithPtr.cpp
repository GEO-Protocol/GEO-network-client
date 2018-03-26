/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "MaxFlowCalculationTrustLineWithPtr.h"

MaxFlowCalculationTrustLineWithPtr::MaxFlowCalculationTrustLineWithPtr(
    const MaxFlowCalculationTrustLine::Shared maxFlowCalculationTrustLine,
    unordered_set<MaxFlowCalculationTrustLineWithPtr*>* hashMapPtr) :

    mMaxFlowCalulationTrustLine(maxFlowCalculationTrustLine),
    mHashSetPtr(hashMapPtr)
{}

MaxFlowCalculationTrustLine::Shared MaxFlowCalculationTrustLineWithPtr::maxFlowCalculationtrustLine()
{
    return mMaxFlowCalulationTrustLine;
}

unordered_set<MaxFlowCalculationTrustLineWithPtr*>* MaxFlowCalculationTrustLineWithPtr::hashSetPtr()
{
    return mHashSetPtr;
}