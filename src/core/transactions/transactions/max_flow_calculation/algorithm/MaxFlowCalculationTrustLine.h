//
// Created by mc on 19.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../trust_lines/TrustLine.h"

class MaxFlowCalculationTrustLine {

public:
    typedef shared_ptr<MaxFlowCalculationTrustLine> Shared;
    typedef shared_ptr<const MaxFlowCalculationTrustLine> SharedConst;

public:
    MaxFlowCalculationTrustLine(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        const TrustLineAmount &amount);

    NodeUUID getSourceUUID();

    NodeUUID getTargetUUID();

    TrustLineAmount getAmount();

    TrustLineAmount getFreeAmount();

    void addUsedAmount(TrustLineAmount amount);

    void setUsedAmount(TrustLineAmount amount);

private:
    NodeUUID mSourceUUID;
    NodeUUID mTargetUUID;
    TrustLineAmount mAmount;
    TrustLineAmount mUsedAmount;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H
