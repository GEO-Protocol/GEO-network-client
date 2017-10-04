#ifndef GEO_NETWORK_CLIENT_BASERESOURCE_H
#define GEO_NETWORK_CLIENT_BASERESOURCE_H

#include "../../common/Types.h"
#include "../../transactions/transactions/base/TransactionUUID.h"

#include <memory>

class BaseResource {
public:
    typedef shared_ptr<BaseResource> Shared;

public:
    enum ResourceType {
        Paths = 1
    };

public:
    BaseResource(
        const ResourceType type,
        const TransactionUUID &transactionUUID);

    const ResourceType type() const;

    const TransactionUUID &transactionUUID() const;

private:
    ResourceType mType;
    TransactionUUID mTransactionUUID;

};


#endif //GEO_NETWORK_CLIENT_BASERESOURCE_H
