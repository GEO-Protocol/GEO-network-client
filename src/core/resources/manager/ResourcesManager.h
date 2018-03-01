#ifndef GEO_NETWORK_CLIENT_RESOURCESMANAGER_H
#define GEO_NETWORK_CLIENT_RESOURCESMANAGER_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"

#include "../resources/BaseResource.h"

#include "../../common/exceptions/ConflictError.h"

#include <boost/signals2.hpp>

using namespace std;
namespace signals = boost::signals2;

class ResourcesManager {
public:
    typedef signals::signal<void(
                const TransactionUUID&,
                const NodeUUID&,
                const SerializedEquivalent)>
            RequestPathsResourcesSignal;
    typedef signals::signal<void(BaseResource::Shared)> AttachResourceSignal;

public:
    void putResource(
        BaseResource::Shared resource);

    void requestPaths(
        const TransactionUUID &transactionUUID,
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent) const;


public:
    mutable RequestPathsResourcesSignal requestPathsResourcesSignal;
    mutable AttachResourceSignal attachResourceSignal;
};


#endif //GEO_NETWORK_CLIENT_RESOURCESMANAGER_H
