#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINE_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINE_H

#include "../common/Types.h"
#include "../trust_lines/TrustLine.h"
#include "../common/time/TimeUtils.h"

class TopologyTrustLine {

public:
    typedef shared_ptr<TopologyTrustLine> Shared;

public:
    TopologyTrustLine(
        ContractorID sourceID,
        ContractorID targetID,
        ConstSharedTrustLineAmount amount);

    const ContractorID sourceID() const;

    const ContractorID targetID() const;

    ConstSharedTrustLineAmount amount() const;

    void setAmount(ConstSharedTrustLineAmount amount);

    ConstSharedTrustLineAmount freeAmount();

    void addUsedAmount(const TrustLineAmount &amount);

    void setUsedAmount(const TrustLineAmount &amount);

private:
    ContractorID mSourceID;
    ContractorID mTargetID;
    ConstSharedTrustLineAmount mAmount;
    SharedTrustLineAmount mUsedAmount;

};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINE_H
