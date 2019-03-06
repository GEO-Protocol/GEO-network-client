#ifndef GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H

#include "../../../SenderMessage.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/memory/MemoryUtils.h"
#include "../../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../../contractors/addresses/BaseAddress.h"

class CycleBaseFiveOrSixNodesInBetweenMessage: public SenderMessage {

public:
    typedef shared_ptr<CycleBaseFiveOrSixNodesInBetweenMessage> Shared;
public:
    CycleBaseFiveOrSixNodesInBetweenMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnReceiverSide,
        vector<BaseAddress::Shared> &path);

    CycleBaseFiveOrSixNodesInBetweenMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

    vector<BaseAddress::Shared> path() const;

protected:
    const size_t kOffsetToInheritedBytes() const override;

protected:
    vector<BaseAddress::Shared> mPath;
};


#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H
