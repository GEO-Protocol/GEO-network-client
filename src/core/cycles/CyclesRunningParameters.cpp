#include "CyclesRunningParameters.h"

CyclesRunningParameters::CyclesRunningParameters() :
    mCyclesThreeNodesEnabled(false),
    mCyclesFourNodesEnabled(false),
    mCyclesFiveNodesEnabled(false),
    mCyclesFiveNodesIntervalSec(0),
    mCyclesSixNodesEnabled(false),
    mCyclesSixNodesIntervalSec(0),
    mRoutingTableUpdatingIntervalDays(0)
{}

CyclesRunningParameters::CyclesRunningParameters(
    bool cyclesThreeNodesEnabled,
    bool cyclesFourNodesEnabled,
    bool cyclesFiveNodesEnabled,
    uint32_t cyclesFiveNodesIntervalSec,
    bool cyclesSixNodesEnabled,
    uint32_t cyclesSixNodesIntervalSec,
    uint16_t routingTableUpdatingIntervalDays):

    mCyclesThreeNodesEnabled(cyclesThreeNodesEnabled),
    mCyclesFourNodesEnabled(cyclesFourNodesEnabled),
    mCyclesFiveNodesEnabled(cyclesFiveNodesEnabled),
    mCyclesFiveNodesIntervalSec(cyclesFiveNodesIntervalSec),
    mCyclesSixNodesEnabled(cyclesSixNodesEnabled),
    mCyclesSixNodesIntervalSec(cyclesSixNodesIntervalSec),
    mRoutingTableUpdatingIntervalDays(routingTableUpdatingIntervalDays)
{}

bool CyclesRunningParameters::isUpdateRoutingTables() const
{
    return mCyclesThreeNodesEnabled or mCyclesFourNodesEnabled;
}

