#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINENEW_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINENEW_H

#include "../common/Types.h"
#include "../trust_lines/TrustLine.h"
#include "../common/time/TimeUtils.h"

class TopologyTrustLineNew {

public:
    typedef shared_ptr<TopologyTrustLineNew> Shared;
    typedef shared_ptr<const TopologyTrustLineNew> SharedConst;

public:
    TopologyTrustLineNew(
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


#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINENEW_H
