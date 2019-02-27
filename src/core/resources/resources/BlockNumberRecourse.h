#ifndef GEO_NETWORK_CLIENT_BLOCKNUMBERRECOURSE_H
#define GEO_NETWORK_CLIENT_BLOCKNUMBERRECOURSE_H

#include "BaseResource.h"

class BlockNumberRecourse : public BaseResource {

public:
    typedef shared_ptr<BlockNumberRecourse> Shared;

public:
    BlockNumberRecourse(
        const TransactionUUID& transactionUUID,
        BlockNumber actualObservingBlockNumber);

    BlockNumber actualObservingBlockNumber() const;

private:
    BlockNumber mActualObservingBlockNumber;
};


#endif //GEO_NETWORK_CLIENT_BLOCKNUMBERRECOURSE_H
