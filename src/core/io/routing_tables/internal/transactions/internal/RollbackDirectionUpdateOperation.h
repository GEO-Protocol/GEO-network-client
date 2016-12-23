#ifndef GEO_NETWORK_CLIENT_ROLLBACKDIRECTIONUPDATEOPERATION_H
#define GEO_NETWORK_CLIENT_ROLLBACKDIRECTIONUPDATEOPERATION_H


#include "Operation.h"
#include "../../../../../common/NodeUUID.h"
#include "../../../../../common/exceptions/RuntimeError.h"


namespace io {
namespace routing_tables {


class RollbackDirectionUpdateOperation:
    public Operation {

public:
    typedef shared_ptr<RollbackDirectionUpdateOperation> Shared;

public:
    explicit RollbackDirectionUpdateOperation(
        const RecordNumber recN,
        TrustLineDirection direction);

    const RecordNumber recordNumber();
    const TrustLineDirection direction() const;

    const pair<shared_ptr<byte>, size_t> serialize() const;

protected:
    RecordNumber mRecN;
    TrustLineDirection mDir;
};


}
}


#endif //GEO_NETWORK_CLIENT_ROLLBACKDIRECTIONUPDATEOPERATION_H
