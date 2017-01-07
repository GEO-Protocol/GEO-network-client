#ifndef GEO_NETWORK_CLIENT_BASEOPERATION_H
#define GEO_NETWORK_CLIENT_BASEOPERATION_H


#include "../../../../../common/Types.h"
#include "../../../../../db/fields/common/AbstractRecordsHandler.h"

#include <utility>
#include <cstring>
#include <memory>
#include "malloc.h"
#include <assert.h>


namespace io {
namespace routing_tables {


using namespace std;
using namespace db;


class Operation:
    public AbstractRecordsHandler {

public:
    typedef shared_ptr<Operation> Shared;
    typedef shared_ptr<const Operation> ConstShared;

    typedef byte SerializedOperationType;
    typedef byte SerializedTrustLineDirectionType;

public:
    enum OperationType {
        Set = 1,
        RollbackSet,

        Remove,
        RollbackRemove,

        Update,
        RollbackUpdate,
    };

public:
    Operation(OperationType type);

    virtual const pair<shared_ptr<byte>, size_t> serialize() const = 0;
    const OperationType type() const;

protected:
    OperationType mType;
};


} // routing_tables
} // io

#endif //GEO_NETWORK_CLIENT_BASEOPERATION_H
