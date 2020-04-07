#ifndef GEO_NETWORK_CLIENT_CYCLESRUNNINGPARAMETERS_H
#define GEO_NETWORK_CLIENT_CYCLESRUNNINGPARAMETERS_H

#include "../common/Types.h"

class CyclesRunningParameters {

public:
    CyclesRunningParameters();

    CyclesRunningParameters(
        bool cyclesThreeNodesEnabled,
        bool cyclesFourNodesEnabled,
        bool cyclesFiveNodesEnabled,
        uint32_t cyclesFiveNodesIntervalSec,
        bool cyclesSixNodesEnabled,
        uint32_t cyclesSixNodesIntervalSec,
        uint16_t routingTableUpdatingIntervalDays);

    bool isUpdateRoutingTables() const;

    bool mCyclesThreeNodesEnabled;
    bool mCyclesFourNodesEnabled;
    bool mCyclesFiveNodesEnabled;
    uint32_t mCyclesFiveNodesIntervalSec;
    bool mCyclesSixNodesEnabled;
    uint32_t mCyclesSixNodesIntervalSec;
    uint16_t mRoutingTableUpdatingIntervalDays;
};


#endif //GEO_NETWORK_CLIENT_CYCLESRUNNINGPARAMETERS_H
