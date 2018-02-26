#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINE_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "../trust_lines/TrustLine.h"
#include "../common/time/TimeUtils.h"

class TopologyTrustLine {

public:
    typedef shared_ptr<TopologyTrustLine> Shared;
    typedef shared_ptr<const TopologyTrustLine> SharedConst;

public:
    TopologyTrustLine(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        ConstSharedTrustLineAmount amount);

    const NodeUUID& sourceUUID() const;

    const NodeUUID& targetUUID() const;

    ConstSharedTrustLineAmount amount() const;

    void setAmount(ConstSharedTrustLineAmount amount);

    ConstSharedTrustLineAmount freeAmount();

    void addUsedAmount(const TrustLineAmount &amount);

    void setUsedAmount(const TrustLineAmount &amount);

private:
    NodeUUID mSourceUUID;
    NodeUUID mTargetUUID;
    ConstSharedTrustLineAmount mAmount;
    SharedTrustLineAmount mUsedAmount;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINE_H
