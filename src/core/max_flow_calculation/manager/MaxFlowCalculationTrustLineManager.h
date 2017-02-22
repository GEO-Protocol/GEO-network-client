//
// Created by mc on 19.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H

#include "../../common/NodeUUID.h"
#include "../MaxFlowCalculationTrustLine.h"

#include <map>
#include <vector>

#include "boost/container/flat_set.hpp"

using namespace std;
class MaxFlowCalculationTrustLineManager {

public:

    void addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine);

// todo make private after testing
public:
    map<NodeUUID, vector<MaxFlowCalculationTrustLine::Shared>> mvTrustLines;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
