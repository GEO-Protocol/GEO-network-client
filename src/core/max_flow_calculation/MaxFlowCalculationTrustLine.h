#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "../trust_lines/TrustLine.h"

class MaxFlowCalculationTrustLine {

public:
    typedef shared_ptr<MaxFlowCalculationTrustLine> Shared;
    typedef shared_ptr<const MaxFlowCalculationTrustLine> SharedConst;

public:
    MaxFlowCalculationTrustLine(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        const TrustLineAmount &amount);

    const NodeUUID& sourceUUID() const;

    const NodeUUID& targetUUID() const;

    const TrustLineAmount& amount() const;

    ConstSharedTrustLineAmount freeAmount() const;

    void addUsedAmount(TrustLineAmount amount);

    void setUsedAmount(TrustLineAmount amount);

private:
    NodeUUID mSourceUUID;
    NodeUUID mTargetUUID;
    TrustLineAmount mAmount;
    TrustLineAmount mUsedAmount;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINE_H
