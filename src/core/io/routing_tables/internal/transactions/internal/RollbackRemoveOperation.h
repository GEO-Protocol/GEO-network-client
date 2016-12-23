#ifndef GEO_NETWORK_CLIENT_ROLLBACKREMOVEOPERATION_H
#define GEO_NETWORK_CLIENT_ROLLBACKREMOVEOPERATION_H


#include "Operation.h"
#include "../../../../../common/NodeUUID.h"
#include "../../../../../common/exceptions/RuntimeError.h"

namespace io {
namespace routing_tables {


class RollbackRemoveOperation:
    public Operation {

public:
    explicit RollbackRemoveOperation(
        const NodeUUID &u1,
        const NodeUUID &u2,
        TrustLineDirection direction);

    const NodeUUID &u1() const;
    const NodeUUID &u2() const;
    const TrustLineDirection direction() const;

    const pair<shared_ptr<byte>, size_t> serialize() const;

protected:
    const NodeUUID mU1;
    const NodeUUID mU2;
    const TrustLineDirection mDir;
};


}
}


#endif //GEO_NETWORK_CLIENT_ROLLBACKREMOVEOPERATION_H
