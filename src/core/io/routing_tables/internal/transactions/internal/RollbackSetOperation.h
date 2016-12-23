#ifndef GEO_NETWORK_CLIENT_ROLLBACKSETOPERATION_H
#define GEO_NETWORK_CLIENT_ROLLBACKSETOPERATION_H


#include "Operation.h"
#include "../../../../../common/NodeUUID.h"
#include "../../../../../common/exceptions/RuntimeError.h"


namespace io {
namespace routing_tables {


class RollbackSetOperation:
    public Operation {

public:
    explicit RollbackSetOperation(
        const NodeUUID &u1,
        const NodeUUID &u2);

    const NodeUUID &u1() const;
    const NodeUUID &u2() const;

    const pair<shared_ptr<byte>, size_t> serialize() const;

protected:
    const NodeUUID mU1;
    const NodeUUID mU2;
};


}
}


#endif //GEO_NETWORK_CLIENT_ROLLBACKSETOPERATION_H
